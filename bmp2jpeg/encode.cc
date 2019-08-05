#include <stdio.h>

#include "bmp.h"
#include "jpeg.h"
#include "huffencode.h"
#include "encode.h"


/*存储像素的RGB以及重新算出的YCbCr值*/
BYTE R[MAX_HEIGHT][MAX_WIDTH];
BYTE G[MAX_HEIGHT][MAX_WIDTH];
BYTE B[MAX_HEIGHT][MAX_WIDTH];
BYTE Y[MAX_HEIGHT][MAX_WIDTH];
BYTE Cr[MAX_HEIGHT/2][MAX_WIDTH/2];
BYTE Cb[MAX_HEIGHT/2][MAX_WIDTH/2];

/*存储BMP头文件中的重要信息*/
MainInfo mi;

/*JPEG文件的输出流*/
OutputStream ops;

/*控制解析整个BMP文件，jpeg和bmp分别是转化前后的文件名*/
bool EncodeWholeFile(char *jpeg, char *bmp)
{
	bool isBMP;         //判断输入文件是否是BMP格式
	bool openJPEG;      //打开JPEG输出文件是否成功
	BMP_Header image;	//存储BMP头文件

	//验证打开的文件是否是BMP格式，并且读入BMP头文件的信息
	isBMP = isBMPPic(&image, bmp);
	if ( !isBMP )
		return false;
	printf("成功读入图片头文件信息\n");

	//获取BMP图像头文件中的一些基本信息
	GetMainInfo(&mi, &image);

	//打开输出的JPEG文件流
	openJPEG = InitJPEG(jpeg, &ops);
	if ( !openJPEG )
		return false;

	//初始化Huffman表
	InitHuffmanTable();

	//读取BMP图像像素RGB值，通过二次取样转化成YCbCr的值
	int width, height;
	RGBToYCrCb(&width, &height);
	printf("成功实现颜色模型转化\n");

	//将头文件信息以及量化表、Huffman表等信息写如JPEG文件中
	writeHeaderTableInfo(&ops,mi.height,mi.width);
    printf("%s %d %d\n",__FUNCTION__,mi.height,mi.width);
    printf("%s %d %d\n",__FUNCTION__,width,height);
	//一次编码16x16块,使色度二次取样后成为8x8块
	int preY = 0;
	int	preCb = 0;
	int preCr = 0;                    //记录上一8*8块的DC值
	int i,j,k,l,m,n;
    BYTE block[DCT_SIZE][DCT_SIZE];   //记录每个8*8块中的Y、Cb、Cr值

	for (i=0; i<height; i+=(DCT_SIZE<<1))
		for (j=0; j<width; j+=(DCT_SIZE<<1))
		{
			//处理每16*16中的4个8*8的亮度块
			for (k=0; k<(DCT_SIZE<<1); k+=DCT_SIZE)
				for (l=0; l<(DCT_SIZE<<1); l+=DCT_SIZE)
				{
					for (m=0; m<DCT_SIZE; m++)
						for (n=0; n<DCT_SIZE; n++)
						{
							block[m][n] = Y[i+k+m][j+l+n];
						}
					EncodeBlock( &ops, &preY, block,lumQuantTable,HuffmanLumDCCode,
						            HuffmanLumDCSize,HuffmanLumACCode, HuffmanLumACSize );
				}

			//处理每16*16中的1个8*8的色度Cb块
			for (m=0; m<DCT_SIZE; m++)
				for (n=0; n<DCT_SIZE; n++)
					block[m][n] = Cb[(i>>1)+m][(j>>1)+n];

			EncodeBlock( &ops, &preCb, block,chrQuantTable,HuffmanChrDCCode,
				            HuffmanChrDCSize,HuffmanChrACCode, HuffmanChrACSize );

			//处理每16*16中的1个8*8的色度Cr块
			for (m=0; m<DCT_SIZE; m++)
				for (n=0; n<DCT_SIZE; n++)
					block[m][n] = Cr[(i>>1)+m][(j>>1)+n];
			EncodeBlock( &ops, &preCr, block,chrQuantTable,HuffmanChrDCCode,
				            HuffmanChrDCSize,HuffmanChrACCode, HuffmanChrACSize );
		}

	//将文件流和文件结尾串写入JPEG文件
	FlushStream(&ops, 1);
	writeEOI(&ops);

	//释放内存空间、关闭文件流
	FreeStream(&ops, 0);
	DeleteMainInfo(&mi);
	CloseBMPHeader(&image);

	return true;
}

/*初始化Huffman表*/
void InitHuffmanTable()
{
	InitHuffTable(  huffmanLumDCBit, huffmanLumDCVal,
					huffmanLumACBit, huffmanLumACVal,
     				huffmanChrDCBit, huffmanChrDCVal,
					huffmanChrACBit, huffmanChrACVal );
}

/*读取BMP图像像素RGB值，通过二次取样转化成YCbCr的值*/
void RGBToYCrCb(int *width, int *height)
{
	//读取BMP图像每个像素RGB值，保存在R、G、B中
	GetRGB(R, G, B, (BYTE *)mi.lpvbits, mi.width, mi.height, mi.bpl);

	//将图片的width和height和均变成16的倍数，方便后面16*16块的划分
	AlignPic(R, G, B, width, height, mi.width, mi.height);

	//将各像素的RGB值转化为YCbCr值
	RGB2YCrCb(Y, Cr, Cb, R, G, B, *width, *height);
}

/*将头文件信息以及量化表、Huffman表等信息写如JPEG文件中*/
void writeHeaderTableInfo(OutputStream *ops,DWORD height,DWORD width)
{
	//将SOI和APP0写入JPEG文件
	writeSOI(ops);
	writeAPP0(ops);

	//将亮度和色度的量化表写入JPEG文件
	writeQuantTable(ops);

  	//将Huffman表写入JPEG文件
	writeHuffmanTable(ops, height,width);
}

/*对被分成8*8小块后的图像进行编码*/
int EncodeBlock(	OutputStream *ops, int *pre, BYTE block[][DCT_SIZE],
      				BYTE quantTable[][DCT_SIZE],
					DWORD *compDCHuffCodePtr, BYTE *compDCHuffSizePtr,
      				DWORD *compACHuffCodePtr, BYTE *compACHuffSizePtr )
{
	int i, j, k;

	double doubleBlock[DCT_SIZE][DCT_SIZE];      //将8*8块中的数据转化成浮点数
	for (i=0; i<DCT_SIZE; i++)
		for (j=0; j<DCT_SIZE; j++)
			doubleBlock[i][j] = (double)block[i][j];

    //进行DCT变化，dct用来记录变化后的DCT系数
	double dct[DCT_SIZE][DCT_SIZE];
	getDCT(dct, doubleBlock);

	//量化DCT系数，quant用来保存量化后的结果
	int quantDCT[DCT_SIZE][DCT_SIZE];
	QuantDCTValue(quantDCT, dct, quantTable);

	/*对每块的DC系数进行Huffman编码*/
	int difDC = quantDCT[0][0]-(*pre);              //pre用来记录编码时前一块的DC值
	(*pre) = quantDCT[0][0];

	//通过查sizeAmplitude表，进行Huffman编码
	int amplitude, size;                             //size和code分别用来该数值在sizeAmplitude表中的两个值
 	VLE(&amplitude, &size, difDC);
	WriteStream(ops, compDCHuffCodePtr[size], compDCHuffSizePtr[size], 1);
 	if (difDC != 0)
  		WriteStream(ops, amplitude, size, 1);


	//对每块的AC系数进行z排序的Huffman编码
	int order[DCT_SIZE*DCT_SIZE];
	ZigZagScan(order, quantDCT);                     //将量化后的数值按照Z排序后保存在order中

	//进行RL编码
	int zlen[DCT_SIZE*DCT_SIZE];
	int num[DCT_SIZE*DCT_SIZE];
	int cnt;
	int val;
	RLE(zlen, num, &cnt, order);

	for (k=0; k<cnt; k++)
	{
		VLE(&amplitude, &size, num[k]);
		val = (zlen[k]<<4)|size;
		WriteStream(ops, compACHuffCodePtr[val], compACHuffSizePtr[val], 1);
		if (zlen[k]!=0 || num[k]!=0)
			WriteStream(ops, amplitude, size, 1);
	}
	return 0;
}







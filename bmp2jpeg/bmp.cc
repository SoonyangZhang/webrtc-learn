#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"

/*读入文件头，确定该文件是否是BMP文件*/
bool isBMPPic(BMP_Header *header, char *BMPfile)
{
	header->fp = fopen(BMPfile, "rb");

	//打开文件失败
	if (header->fp == NULL)
		return false;

	//将指针定位到文件开始位置
	fseek(header->fp, 0, SEEK_SET);

	//分别读入文件的FILEHEADER和INFOHEADER
	fread(&(header->fileHeader), sizeof(BITMAPFILEHEADER), 1, header->fp);
	fread(&(header->infoHeader), sizeof(BITMAPINFOHEADER), 1, header->fp);

	//根据FILEHEADER的信息，判断该文件是否是BMP文件
	if (strncmp((char *)&(header->fileHeader.bfType), "BM", 2) != 0)
	{
		fclose(header->fp);
		return false;
	}
	return true;
}

/*根据BMP图像中infoHeader和fileHeader中的内容，获取BMP图像的一些基本信息*/
void GetMainInfo(MainInfo *mi, BMP_Header *image)
{
	WORD n;
	mi->width = image->infoHeader.biWidth;
	mi->height = image->infoHeader.biHeight;
	mi->bitCount = image->infoHeader.biBitCount;
        printf("%s %d %d\n",__FUNCTION__,mi->width,mi->height);
	n = mi->width*mi->bitCount;
	if (n%32 == 0)
		mi->bpl = n/8;
	else
  		mi->bpl = n/8+(4-(n/8)%4);
	if (mi->bitCount <= 8)
		mi->palCount = 1<<(mi->bitCount);
	else
		mi->palCount = 0;

	mi->picSize = mi->bpl*mi->height;
	mi->offSet = image->fileHeader.bfOffBits-14;

 	mi->lpbmi = (LPBITMAPINFO)malloc(40+sizeof(RGBQUAD)*mi->palCount+mi->picSize);
 	mi->lpvbits = (BYTE *)mi->lpbmi+mi->offSet;

 	mi->lpvbits = (BYTE *)mi->lpbmi+mi->offSet;

 	memcpy(mi->lpbmi, &image->infoHeader, 40);
 	if (mi->palCount > 0)
 	{
 		fseek(image->fp, 14+40, SEEK_SET);
	 	fread((BYTE *)mi->lpbmi+40, sizeof(RGBQUAD), mi->palCount, image->fp);
	}
 	fseek(image->fp, 14+mi->offSet, SEEK_SET);
 	fread(mi->lpvbits, sizeof(BYTE), mi->picSize, image->fp);
}

/*读取BMP图片中每个像素的RGB值*/
void GetRGB( BYTE R[][MAX_WIDTH], BYTE G[][MAX_WIDTH], BYTE B[][MAX_WIDTH],BYTE BGR[], int width, int height, int bpl )
{
	int i, j, t1, t2;
	for (i=0; i<height; i++)
	{
		t1 = height-i-1;
		for (j=0; j<width; j++)
		{
			t2 = i*bpl+j*3;
			R[t1][j] = BGR[t2+2];
			G[t1][j] = BGR[t2+1];
			B[t1][j] = BGR[t2];
 		}
	}
}

/*通过边缘扩充的方法，将图片的width和height和均变成16的倍数*/
void AlignPic( BYTE R[][MAX_WIDTH], BYTE G[][MAX_WIDTH], BYTE B[][MAX_WIDTH],int *wid, int *hei, int width, int height)
{
	int i, j;
	if (width%16==0 && height%16==0)
	{
		*wid = width;
		*hei = height;
		return;
	}
	if (width%16 == 0)
		*wid = width;
	else
		*wid = width+(16-width%16);
	if (height%16 == 0)
		*hei = height;
	else
		*hei = height+(16-height%16);

	//用边缘扩充的办法填充
	for (i=0; i<height; i++)
		for (j=width; j<*wid; j++)
		{
			R[i][j] = R[i][width-1];
			G[i][j] = G[i][width-1];
			B[i][j] = B[i][width-1];
  		}

	//用边缘扩充的办法填充
	for (j=0; j<*wid; j++)
		for (i=height; i<*hei; i++)
		{
			R[i][j] = R[height-1][j];
			G[i][j] = G[height-1][j];
			B[i][j] = B[height-1][j];
  		}
}

/*确保Y、Cr和Cb的值在0-255之间*/
inline BYTE EnsureByte(double val)
{
	int ival = (int)val;
	if (ival < 0)
		return 0;
	if (ival > 255)
		return 255;
	return (BYTE)ival;
}

/*将各像素的RGB值转化为YCbCr值*/
int RGB2YCrCb(BYTE Y[][MAX_WIDTH], BYTE Cr[][MAX_WIDTH/2], BYTE Cb[][MAX_WIDTH/2],BYTE R[][MAX_WIDTH], BYTE G[][MAX_WIDTH], BYTE B[][MAX_WIDTH],int width, int height )
{
	int i, j;
	if (width%2!=0 || height%2!=0)
		return -1;
	for (i=0; i<height; i++)
		for (j=0; j<width; j++)
  			Y[i][j] = EnsureByte(0.299*R[i][j] + 0.587*G[i][j] + 0.114*B[i][j]);
	for (i=0; i<height; i+=2)
		for (j=0; j<width; j+=2)
		{
   			Cr[i>>1][j>>1] = EnsureByte(0.5*R[i][j] - 0.4187*G[i][j] - 0.0813*B[i][j] + 128);
  			Cb[i>>1][j>>1] = EnsureByte(-0.1687*R[i][j] - 0.3313*G[i][j] + 0.5*B[i][j] + 128);
  		}
	return 0;
}



/*释放基本信息存储的空间*/
void DeleteMainInfo(MainInfo *mi)
{
	free(mi->lpbmi);
	//mi->hDIB = NULL;
}


/*关闭BMP文件*/
void CloseBMPHeader(BMP_Header *image)
{
	fclose(image->fp);
}

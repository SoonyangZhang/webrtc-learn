#include <stdio.h>
#include <math.h>
#include "huffencode.h"


/*Z型扫描的顺序表*/
BYTE zigZagTable[DCT_SIZE][DCT_SIZE] = {
							{ 0, 1, 5, 6, 14,15,27,28 },
							{ 2, 4, 7, 13,16,26,29,42 },
							{ 3, 8, 12,17,25,30,41,43 },
							{ 9, 11,18,24,31,40,44,53 },
							{ 10,19,23,32,39,45,52,54 },
							{ 20,22,33,38,46,51,55,60 },
							{ 21,34,37,47,50,56,59,61 },
							{ 35,36,48,49,57,58,62,63 } };
//所有HuffmanBitDef中的[0]均不使用

/*默认的huffman亮度位数和DC系数表*/
BYTE huffmanLumDCBit[17] = { 0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 };
BYTE huffmanLumDCVal[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

/*默认的huffman色度位数和DC系数表*/
BYTE huffmanChrDCBit[17] = { 0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };
BYTE huffmanChrDCVal[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

/*默认的huffman亮度位数和AC系数表*/
BYTE huffmanLumACBit[17] =  { 0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d };
BYTE huffmanLumACVal[162] = {
				0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
				0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
				0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
				0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
				0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
				0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
				0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
				0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
				0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
				0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
				0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
				0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
				0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
				0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
				0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
				0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
				0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
				0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
				0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
				0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,0xf8,
				0xf9, 0xfa };


/*默认的huffman色度位数和AC系数表*/
BYTE huffmanChrACBit[17] =  { 0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 };
BYTE huffmanChrACVal[162] = {
				0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
				0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
				0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
				0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
				0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
				0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
				0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
				0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
				0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
				0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
				0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
				0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
				0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
				0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
				0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
				0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
				0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
				0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
				0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
				0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
				0xf9, 0xfa };

/*定义SizeAmplitude表*/
BYTE sizeAmplitudeTable[1<<15];

/*定义Huffman码字*/
DWORD HuffmanLumDCCode[256], HuffmanLumACCode[256];
BYTE HuffmanLumDCSize[256], HuffmanLumACSize[256];

DWORD HuffmanChrDCCode[256], HuffmanChrACCode[256];
BYTE HuffmanChrDCSize[256], HuffmanChrACSize[256];


/*对8*8块进行Z型扫描，将quant按照z字形排列存入order*/
void ZigZagScan(int order[], int quant[][DCT_SIZE])
{
	int i, j;
	for (i=0; i<DCT_SIZE; i++)
		for (j=0; j<DCT_SIZE; j++)
			order[zigZagTable[i][j]] = quant[i][j];
}

/*进行RLE编码，zlen代表0的个数，num代表非0数值，cnt代表RLE的个数 */
void RLE(int zlen[], int num[], int *cnt, int order[])
{
	int k = 0, z = 0;
	(*cnt) = 0;
 	do {
 		k++;
 		if (order[k] == 0)
 		{
			//0,0代表一块的结束
 			if (k == 63)
 			{
 				zlen[*cnt] = 0;
 				num[*cnt] = 0;
 				(*cnt)++;
			}
			else
				z++;
		}
		else
		{
			//0的个数不能大于15
			while (z > 15)
			{
				zlen[*cnt] = 15;
				num[*cnt] = 0;
				(*cnt)++;
				z -= 16;
    		}
			zlen[*cnt] = z;
			num[*cnt] = order[k];
			(*cnt)++;
			z = 0;
  		}
  	} while(k < 63);
}

/*初始化查找表*/
void InitSizeAmplitudeTable()
{
	int i, j, lastpw = 1, pw = 1;
	sizeAmplitudeTable[0] = 0;
	for (i=1; i<16; i++)
	{
		pw *= 2;
		for (j=lastpw; j<pw; j++)
			sizeAmplitudeTable[j] = i;
		lastpw = pw;
	}
}


/*计算某个数值在sizeAmplitude表中的两个数值*/
void VLE(int *amplitude, int *size, int val)
{
	int aval = abs(val);
	*size = sizeAmplitudeTable[aval];
	if (val >= 0)
		*amplitude = val;
	else
		*amplitude = (1<<(*size))-1-aval;
}


/*生成Huffman表*/
void MakeHuffTable(DWORD code[], BYTE size[], BYTE bit[], BYTE val[])
{
	int i, j, k = 0;
	DWORD mode = 0;
	DWORD tmpcode[162];
	BYTE tmpsize[162];
	for (i=0; i<256; i++)
	{
		code[i] = 0;
		size[i] = 0;
	}
	for (i=1; i<=16; i++)
	{
		if (bit[i] > 0)
			for (j=0; j<bit[i]; j++)
			{
				tmpsize[k] = (unsigned char)i;
				tmpcode[k] = mode;
				mode++;
				k++;
			}
		mode <<= 1;
	}
	for (i=0; i<k; i++)
	{
		code[val[i]] = tmpcode[i];
  		size[val[i]] = tmpsize[i];
 	}
}


/*初始化需要的Huffman表和SizeAmplitude表*/
int InitHuffTable( 	BYTE *lumDCHuffBitPtr, BYTE *lumDCHuffValPtr,
					BYTE *lumACHuffBitPtr, BYTE *lumACHuffValPtr,
   					BYTE *chrDCHuffBitPtr, BYTE *chrDCHuffValPtr,
      				BYTE *chrACHuffBitPtr, BYTE *chrACHuffValPtr )
{

	MakeHuffTable(HuffmanLumDCCode, HuffmanLumDCSize, lumDCHuffBitPtr, lumDCHuffValPtr);
	MakeHuffTable(HuffmanLumACCode, HuffmanLumACSize, lumACHuffBitPtr, lumACHuffValPtr);
	MakeHuffTable(HuffmanChrDCCode, HuffmanChrDCSize, chrDCHuffBitPtr, chrDCHuffValPtr);
	MakeHuffTable(HuffmanChrACCode, HuffmanChrACSize, chrACHuffBitPtr, chrACHuffValPtr);

	InitSizeAmplitudeTable();

	return 0;
}







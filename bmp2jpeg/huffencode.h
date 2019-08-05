#ifndef	HUFFENCODE_H
#define	HUFFENCODE_H

#include "datatype.h"
#include "dct.h"

extern BYTE zigZagTable[DCT_SIZE][DCT_SIZE];
extern BYTE huffmanLumDCBit[17];
extern BYTE huffmanLumDCVal[12];
extern BYTE huffmanLumACBit[17];
extern BYTE huffmanLumACVal[162];

extern BYTE huffmanChrDCBit[17];
extern BYTE huffmanChrDCVal[12];
extern BYTE huffmanChrACBit[17];
extern BYTE huffmanChrACVal[162];

extern DWORD HuffmanLumDCCode[256], HuffmanLumACCode[256];
extern BYTE HuffmanLumDCSize[256], HuffmanLumACSize[256];

extern DWORD HuffmanChrDCCode[256], HuffmanChrACCode[256];
extern BYTE HuffmanChrDCSize[256], HuffmanChrACSize[256];

/*对8*8块进行Z型扫描*/
void ZigZagScan(int scan[], int quant[][DCT_SIZE]);

/*进行RLE编码*/
void RLE(int zlen[], int num[], int *cnt, int scan[]);

/*初始化查找表*/
void InitSizeAmplitudeTable();

/*计算某个数值在sizeAmplitude表中的两个数值*/
void VLE(int *code, int *group, int val);

/*生成Huffman表*/
void MakeHuffTable(unsigned int code[], unsigned char size[], BYTE bit[], BYTE val[]);

/*初始化需要的Huffman表和SizeAmplitude表*/
int InitHuffTable(BYTE *lumDCHuffBitPtr, BYTE *lumDCHuffValPtr,BYTE *lumACHuffBitPtr, BYTE *lumACHuffValPtr,BYTE *chrDCHuffBitPtr, BYTE *chrDCHuffValPtr,BYTE *chrACHuffBitPtr, BYTE *chrACHuffValPtr );

#endif

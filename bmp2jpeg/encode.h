#ifndef Enode_H
#define Enode_H

#include "dct.h"
#include "io.h"
#include "bmp.h"

/*将图像分成8*8小块后，进行编码*/
int EncodeBlock( OutputStream *strm, int *preCompDC, BYTE block[][DCT_SIZE],
      			BYTE compQuantTablePtr[][DCT_SIZE],
				DWORD *compDCHuffCodePtr, BYTE *compDCHuffSizePtr,
      			DWORD *compACHuffCodePtr, BYTE *compACHuffSizePtr );

/*初始化Huffman表*/
void InitHuffmanTable();

/*读取BMP图像像素RGB值，通过二次取样转化成YCbCr的值*/
void RGBToYCrCb(int *width, int *height);

/*将头文件信息以及量化表、Huffman表等信息写如JPEG文件中*/
void writeHeaderTableInfo(OutputStream *ops,DWORD height,DWORD width);

/*控制解析整个BMP文件*/
bool EncodeWholeFile(char *destName, char *srcName);

#endif

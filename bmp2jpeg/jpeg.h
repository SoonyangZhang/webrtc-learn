#ifndef	JPEG_H
#define	JPEG_H

#include "datatype.h"
#include "io.h"
#include "bmp.h"

/*定义JPEG文件标识信息*/
#define MARKER_SOI	1  //文件开始标志
#define MARKER_APP0	2  //JFIF(JPEG File Interchange Format)标志
#define MARKER_DQT	3  //量化表
#define MARKER_SOF0	4  //第0个frame开始标志
#define MARKER_DHT	5  //霍夫曼表
#define MARKER_SOS	6  //start of scan
#define MARKER_EOI	7  //文件结束标志

static BYTE SOI[2]   = { 0xff, 0xd8 };
static BYTE APP0[18] = { 0xff, 0xe0, 0x00, 0x10, 0x4A,
 						     0x46, 0x49, 0x46, 0x00, 0x01,
      						 0x01, 0x00, 0x00, 0x01, 0x00,
   		   					 0x01, 0x00, 0x00 };
static BYTE DQT[5]   = { 0xff, 0xdb, 0x00, 0x43, 0 };
static BYTE SOF0[19] = { 0xff, 0xc0, 0x00, 0x11, 0x08,
 							 0,    0,    0,    0,
           					 0x03, 0x01, 0x22, 0x00, 0x02,
               			     0x11, 0x01, 0x03, 0x11, 0x01 };
static BYTE DHT[5]   = { 0xff, 0xc4, 0,    0,    0 };
static BYTE SOS[14]  = { 0xff, 0xda, 0x00, 0x0c, 0x03,
 				             0x01, 0x00, 0x02, 0x11, 0x03,
      				         0x11, 0x00, 0x3f, 0x00 };
static BYTE EOI[2]   = { 0xff, 0xd9 };

/*将SOI写入JPEG文件*/
void writeSOI(OutputStream *ops);

/*将APP0写入JPEG文件*/
void writeAPP0(OutputStream *ops);

/*将亮度和色度量化表写入JPEG文件*/
void writeQuantTable(OutputStream *ops);

/*将Huffman表写入JPEG文件*/
void writeHuffmanTable(OutputStream *ops, MainInfo *mi);

/*将EOI写入JPEG文件*/
void writeEOI(OutputStream *ops);

#endif

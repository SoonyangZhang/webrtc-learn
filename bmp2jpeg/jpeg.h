#ifndef	JPEG_H
#define	JPEG_H

#include "datatype.h"
#include "io.h"
#include "bmp.h"

/*����JPEG�ļ���ʶ��Ϣ*/
#define MARKER_SOI	1  //�ļ���ʼ��־
#define MARKER_APP0	2  //JFIF(JPEG File Interchange Format)��־
#define MARKER_DQT	3  //������
#define MARKER_SOF0	4  //��0��frame��ʼ��־
#define MARKER_DHT	5  //��������
#define MARKER_SOS	6  //start of scan
#define MARKER_EOI	7  //�ļ�������־

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

/*��SOIд��JPEG�ļ�*/
void writeSOI(OutputStream *ops);

/*��APP0д��JPEG�ļ�*/
void writeAPP0(OutputStream *ops);

/*�����Ⱥ�ɫ��������д��JPEG�ļ�*/
void writeQuantTable(OutputStream *ops);

/*��Huffman��д��JPEG�ļ�*/
void writeHuffmanTable(OutputStream *ops, MainInfo *mi);

/*��EOIд��JPEG�ļ�*/
void writeEOI(OutputStream *ops);

#endif
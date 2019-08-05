#ifndef IO_H
#define IO_H

#include <stdio.h>
#include "datatype.h"

/*����������ĸ�ʽ*/
typedef struct
{
 	DWORD buffer; 
	BYTE ptr;	   
	FILE *fp;
}OutputStream;

/*��ʼ��JPEG�ļ��������*/
bool InitJPEG(char *desFile, OutputStream *stream);

/* ��ops�е�����ֱ��д��buffer�����ݵĳ���Ϊlength*/
int WriteDirect(OutputStream *strm, BYTE *buffer, DWORD length);

/*�����Ȳ�ȷ��������д��buffer��*/
void WriteStream(OutputStream *strm, DWORD code, BYTE length, int flag);

/*��ʣ���opsд�����ļ�*/
void FlushStream(OutputStream *strm, int flag);

/*�ͷ�д����ops*/
void FreeStream(OutputStream *strm, int flag);

#endif
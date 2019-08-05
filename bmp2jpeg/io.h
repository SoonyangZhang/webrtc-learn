#ifndef IO_H
#define IO_H

#include <stdio.h>
#include "datatype.h"

/*定义输出流的格式*/
typedef struct
{
 	DWORD buffer; 
	BYTE ptr;	   
	FILE *fp;
}OutputStream;

/*初始化JPEG文件的输出流*/
bool InitJPEG(char *desFile, OutputStream *stream);

/* 把ops中的数据直接写入buffer，数据的长度为length*/
int WriteDirect(OutputStream *strm, BYTE *buffer, DWORD length);

/*将长度不确定的内容写入buffer中*/
void WriteStream(OutputStream *strm, DWORD code, BYTE length, int flag);

/*将剩余的ops写出到文件*/
void FlushStream(OutputStream *strm, int flag);

/*释放写入流ops*/
void FreeStream(OutputStream *strm, int flag);

#endif

#include <stdio.h>
#include "io.h"

/*初始化JPEG文件的输出流*/
bool InitJPEG(char *desFile, OutputStream *stream)
{
	//给JPEG文件输出流中的相关参数赋值
	stream->fp = fopen(desFile, "wb");
	if (stream->fp == NULL)
		return false;

	fseek(stream->fp, 0, SEEK_SET);
	stream->buffer = 0;
	stream->ptr = 0;
	return true;
}

/* 把ops中的数据直接写入buffer，数据的长度为length*/
int WriteDirect(OutputStream *ops, BYTE *buffer, DWORD length)
{
	if (ops->ptr != 0)
		return -1;
	fwrite(buffer, 1, length, ops->fp);
	return 0;
}

/*将长度不确定的内容写入buffer中*/
void WriteStream(OutputStream *ops, DWORD code, BYTE length, int flag)
{
	int i, k, len = length;
	BYTE fillbyte = 0;
	while (len > 0)
	{
		//写4个字节
		while (ops->ptr == 32)
		{
  			for (i=3; i>=0; i--)
  			{
  				fwrite(((BYTE *)&(ops->buffer))+i, sizeof(BYTE), 1, ops->fp);
  				ops->ptr -= 8;
  				if (flag)
  				{
  					if (*(((BYTE *)&(ops->buffer))+i) == 0xff)
  						fwrite(&fillbyte, sizeof(BYTE), 1, ops->fp);
				}
			}
			ops->buffer = 0;
		}
		if (len <= 32-ops->ptr)
			k = len;
		else
			k = 32-ops->ptr;
		ops->buffer <<= k;
		ops->buffer |= (code>>(len-k))&((1<<k)-1);
		ops->ptr += k;
		len -= k;
	}
}

/*将剩余的ops写出到文件*/
void FlushStream(OutputStream *ops, int flag)
{
	int i;
	BYTE fillbyte = 0;
	while (ops->ptr%8 != 0)
		WriteStream(ops, 1, 1, 1);
	for (i=(ops->ptr)/8-1; i>=0; i--)
  	{
  		fwrite(((BYTE *)&(ops->buffer))+i, sizeof(BYTE), 1, ops->fp);
  		ops->ptr -= 8;
  		if (flag)
  		{
  			if (*(((BYTE *)&(ops->buffer))+i) == 0xff)
  				fwrite(&fillbyte, sizeof(BYTE), 1, ops->fp);
		}
	}
	ops->buffer = 0;
}

/*释放写入流ops*/
void FreeStream(OutputStream *ops, int flag)
{
	if (ops->ptr > 0)
		FlushStream(ops, flag);
	fclose(ops->fp);
}



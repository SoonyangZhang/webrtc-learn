#include <stdio.h>
#include "io.h"

/*��ʼ��JPEG�ļ��������*/
bool InitJPEG(char *desFile, OutputStream *stream)
{
	//��JPEG�ļ�������е���ز�����ֵ
	stream->fp = fopen(desFile, "wb");
	if (stream->fp == NULL)
		return false;

	fseek(stream->fp, 0, SEEK_SET);
	stream->buffer = 0;
	stream->ptr = 0;
	return true;
}

/* ��ops�е�����ֱ��д��buffer�����ݵĳ���Ϊlength*/
int WriteDirect(OutputStream *ops, BYTE *buffer, DWORD length)
{
	if (ops->ptr != 0)
		return -1;
	fwrite(buffer, 1, length, ops->fp);
	return 0;
}

/*�����Ȳ�ȷ��������д��buffer��*/
void WriteStream(OutputStream *ops, DWORD code, BYTE length, int flag)
{
	int i, k, len = length;
	BYTE fillbyte = 0;
	while (len > 0)
	{
		//д4���ֽ�
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

/*��ʣ���opsд�����ļ�*/
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

/*�ͷ�д����ops*/
void FreeStream(OutputStream *ops, int flag)
{
	if (ops->ptr > 0)
		FlushStream(ops, flag);
	fclose(ops->fp);
}


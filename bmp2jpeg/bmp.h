#ifndef	BMP_H
#define	BMP_H
#include "datatype.h"
#include <stdint.h>
#if defined linux_plat
# pragma pack (2)
typedef struct tagBITMAPFILEHEADER {
	WORD  bfType;
	DWORD bfSize;
	WORD  bfReserved1;
	WORD  bfReserved2;
	DWORD bfOffBits;
} BITMAPFILEHEADER, *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;;
//# pragma pack (2) that's the reason
typedef struct tagBITMAPINFOHEADER {
	DWORD biSize;
	LONG  biWidth;
	LONG  biHeight;
	WORD  biPlanes;
	WORD  biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG  biXPelsPerMeter;
	LONG  biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;;
typedef struct tagRGBQUAD
{ BYTE				rgbBlue;
  BYTE				rgbGreen;
  BYTE				rgbRed;
  BYTE				rgbReserved;
} RGBQUAD, *LPRGBQUAD;
typedef struct tagBITMAPINFO
{ BITMAPINFOHEADER		bmiHeader;
  RGBQUAD			bmiColors[1];
} BITMAPINFO, *LPBITMAPINFO, *PBITMAPINFO;
#endif
/*�洢BMPͼ���һЩ������Ϣ*/
typedef	struct
{
	WORD	bitCount;
	WORD	bpl;
	DWORD	height;
	DWORD	width;
	DWORD	palCount;
	DWORD	picSize;
	DWORD	offSet;
	//HANDLE	hDIB;
	LPBITMAPINFO	lpbmi;
	void*	lpvbits;
}MainInfo;

/*�洢BMP�ļ�ͷ*/
typedef struct
{
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;
	FILE *fp;
}BMP_Header;

/*����ת��ͼƬ������������߶�*/
#define MAX_WIDTH	1500
#define MAX_HEIGHT	1500

/*����BMP�ļ�ͷ��ȷ�����ļ��Ƿ���BMP�ļ�*/
bool isBMPPic(BMP_Header *header, char *BMPfile);

/*����BMPͼ����infoHeader��fileHeader�е����ݣ���ȡBMPͼ���һЩ������Ϣ*/
void GetMainInfo(MainInfo *mi, BMP_Header *image);

/*�������ص�RGBֵת��ΪYCbCrֵ*/
int RGB2YCrCb(	BYTE Y[][MAX_WIDTH], BYTE Cb[][MAX_WIDTH/2], BYTE Cr[][MAX_WIDTH/2],BYTE R[][MAX_WIDTH], BYTE G[][MAX_WIDTH], BYTE B[][MAX_WIDTH],int width, int height );

/*ͨ����Ե����ķ�������ͼƬ��width��height�;����16�ı���*/
void AlignPic( BYTE R[][MAX_WIDTH], BYTE G[][MAX_WIDTH], BYTE B[][MAX_WIDTH],int *wid, int *hei, int width, int height);

/*��ȡBMPͼƬ��ÿ�����ص�RGBֵ*/
void GetRGB( BYTE R[][MAX_WIDTH], BYTE G[][MAX_WIDTH], BYTE B[][MAX_WIDTH],BYTE BGR[], int width, int height, int bpl );

/*�ͷŻ�����Ϣ�洢�Ŀռ�*/
void DeleteMainInfo(MainInfo *mi);

/*�ر�BMP�ļ�*/
void CloseBMPHeader(BMP_Header *image);

#endif

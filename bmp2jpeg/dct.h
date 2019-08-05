#ifndef	DCT_H
#define	DCT_H

#include "datatype.h"

/*DCT�������ĸ���*/
#define DCT_SIZE	8

/*���ȵ�������*/
extern BYTE lumQuantTable[DCT_SIZE][DCT_SIZE];

/*ɫ�ȵ�������*/
extern BYTE chrQuantTable[DCT_SIZE][DCT_SIZE];

/*DCT�任����,������AAN�����Ż�*/
void DCT_AAN(double dct[], double pic[]);

/*����DCT�仯*/
void getDCT(double dct[][DCT_SIZE], double pic[][DCT_SIZE]);

/*��DCT��ֵ��������*/
void QuantDCTValue(int quant[][DCT_SIZE], double dct[][DCT_SIZE], BYTE compQuantTable[][DCT_SIZE]);

#endif
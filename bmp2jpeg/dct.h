#ifndef	DCT_H
#define	DCT_H

#include "datatype.h"

/*DCT基函数的个数*/
#define DCT_SIZE	8

/*亮度的量化表*/
extern BYTE lumQuantTable[DCT_SIZE][DCT_SIZE];

/*色度的量化表*/
extern BYTE chrQuantTable[DCT_SIZE][DCT_SIZE];

/*DCT变换函数,利用了AAN进行优化*/
void DCT_AAN(double dct[], double pic[]);

/*进行DCT变化*/
void getDCT(double dct[][DCT_SIZE], double pic[][DCT_SIZE]);

/*将DCT数值进行量化*/
void QuantDCTValue(int quant[][DCT_SIZE], double dct[][DCT_SIZE], BYTE compQuantTable[][DCT_SIZE]);

#endif

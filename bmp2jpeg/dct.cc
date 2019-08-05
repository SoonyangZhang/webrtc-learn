#include <stdio.h>
#include "dct.h"

/*亮度量化表*/
BYTE lumQuantTable[DCT_SIZE][DCT_SIZE] = {
						{ 16, 11, 10, 16, 24, 40, 51, 61 },
    					{ 12, 12, 14, 19, 26, 58, 60, 55 },
						{ 14, 13, 16, 24, 40, 57, 69, 56 },
						{ 14, 17, 22, 29, 51, 87, 80, 62 },
						{ 18, 22, 37, 56, 68, 109,103,77 },
						{ 24, 35, 55, 64, 81, 104,113,92 },
						{ 49, 64, 78, 87, 103,121,120,101},
						{ 72, 92, 95, 98, 112,100,103,99 } };

/*色度量化表*/
BYTE chrQuantTable[DCT_SIZE][DCT_SIZE] = {
						{ 17,18,24,47,99,99,99,99 },
						{ 18,21,26,66,99,99,99,99 },
						{ 24,26,56,99,99,99,99,99 },
						{ 47,66,99,99,99,99,99,99 },
						{ 99,99,99,99,99,99,99,99 },
						{ 99,99,99,99,99,99,99,99 },
						{ 99,99,99,99,99,99,99,99 },
						{ 99,99,99,99,99,99,99,99 } };


/*DCT变换函数,利用了AAN进行优化*/
void DCT_AAN(double dct[], double original[])
{
	static double x01, x11, x21, x31, x41, x51, x61, x71;
	static double x0, x1, x2, x3;
	x01 = original[0]+original[7];
	x11 = original[1]+original[6];
	x21 = original[2]+original[5];
	x31 = original[3]+original[4];
	x41 = original[3]-original[4];
	x51 = original[2]-original[5];
	x61 = original[1]-original[6];
	x71 = original[0]-original[7];

	x0 = x01+x31;
	x1 = x11+x21;
	x2 = x11-x21;
	x3 = x01-x31;

	dct[0] = 0.35355339*(x0+x1);
	dct[4] = 0.35355339*(x0-x1);
	x2 = 0.707106781*(x2+x3);
	dct[2] = 0.27059805*(x2+x3);
	dct[6] = 0.653281482*(x3-x2);

	x0 = x41+x51;
	x1 = x51+x61;
	x2 = x61+x71;
	x3 = 0.382683432*(x0-x2);

	x41 = 0.541196100*x0+x3;
	x51 = 0.707106781*x1;
	x61 = 1.306562965*x2+x3;

	x1 = x51+x71;
	x3 = x71-x51;
	dct[5] = 0.449988111*(x41+x3);
	dct[1] = 0.254897789*(x1+x61);
	dct[7] = 1.281457724*(x1-x61);
	dct[3] = 0.300672443*(x3-x41);
}

/*进行DCT变化，结果存储在dct中*/
void getDCT(double dct[][DCT_SIZE], double original[][DCT_SIZE])
{
	static int i, j;
	static double res[DCT_SIZE][DCT_SIZE], out[DCT_SIZE], in[DCT_SIZE];
	for (i=0; i<DCT_SIZE; i++)
	{
		for (j=0; j<DCT_SIZE; j++)
			in[j] = original[i][j]-128;	   //将范围从0-255 变到-128到127

		 //计算一维DCT,结果存入RES
 		DCT_AAN(res[i], in);
	}

	for (i=0; i<DCT_SIZE; i++)
	{
		for (j=0; j<DCT_SIZE; j++)
			in[j] = res[j][i];
		DCT_AAN(out, in);
		for (j=0; j<DCT_SIZE; j++)
			dct[j][i] = out[j];
	}
}

/*将DCT变化后的系数进行量化*/
void QuantDCTValue(int quant[][DCT_SIZE], double dct[][DCT_SIZE], BYTE quantTable[][DCT_SIZE])
{
	int i, j;
	for (i=0; i<DCT_SIZE; i++)
		for (j=0; j<DCT_SIZE; j++)
		{
			if(dct[i][j] >= 0)
				quant[i][j] = (int)(dct[i][j]/quantTable[i][j]+0.5);
			else
				quant[i][j] = (int)(dct[i][j]/quantTable[i][j]-0.5);
		}
}

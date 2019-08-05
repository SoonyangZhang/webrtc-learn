/*
https://blog.csdn.net/ctfysj/article/details/81299473
*/
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#define WIDTH 1280
#define HEIGHT 720
//转换矩阵
double YuvToRgb[3][3] = {1,       0,  1.4022,
                         1,    -0.3456, -0.7145,
                         1,   1.771,       0};

//根据RGB三分量写BMP，不必关注
int WriteBmp(int width, int height, unsigned char *R,unsigned char *G,unsigned char *B, char *BmpFileName);

//转换函数
int Convert(char *file, int width, int height, int n)
{
    //变量声明
    int i = 0;
    int temp = 0;
    int x = 0;
    int y = 0;
    int fReadSize = 0;
    int ImgSize = width*height;
    FILE *fp = NULL;
    unsigned char* yuv = NULL;
    unsigned char* rgb = NULL;
    unsigned char* cTemp[6];
    char BmpFileName[256];

    //申请空间
    int FrameSize = ImgSize + (ImgSize >> 1);
    yuv = (unsigned char *)malloc(FrameSize);
    rgb = (unsigned char *)malloc(ImgSize*3);
    //读取指定文件中的指定帧
    if((fp = fopen(file, "rb")) == NULL)
        return 0;
    fseek(fp, FrameSize*(n-1), SEEK_CUR);
    fReadSize = fread(yuv, 1, FrameSize, fp);
    fclose(fp);
    if(fReadSize < FrameSize)
        return 0;
    //转换指定帧  如果你不是处理文件 主要看这里
    cTemp[0] = yuv;                        //y分量地址
    cTemp[1] = yuv + ImgSize;            //u分量地址
    cTemp[2] = cTemp[1] + (ImgSize>>2);    //v分量地址
    cTemp[3] = rgb;                        //r分量地址
    cTemp[4] = rgb + ImgSize;            //g分量地址
    cTemp[5] = cTemp[4] + ImgSize;        //b分量地址
    for(y=0; y < height; y++)
        for(x=0; x < width; x++)
        {
            //r分量
            temp = cTemp[0][y*width+x] + (cTemp[2][(y/2)*(width/2)+x/2]-128) * YuvToRgb[0][2];
            cTemp[3][y*width+x] = temp<0 ? 0 : (temp>255 ? 255 : temp);
            //g分量
            temp = cTemp[0][y*width+x] + (cTemp[1][(y/2)*(width/2)+x/2]-128) * YuvToRgb[1][1]
                                       + (cTemp[2][(y/2)*(width/2)+x/2]-128) * YuvToRgb[1][2];
            cTemp[4][y*width+x] = temp<0 ? 0 : (temp>255 ? 255 : temp);
            //b分量
            temp = cTemp[0][y*width+x] + (cTemp[1][(y/2)*(width/2)+x/2]-128) * YuvToRgb[2][1];
            cTemp[5][y*width+x] = temp<0 ? 0 : (temp>255 ? 255 : temp);
        }

    //写到BMP文件中
    sprintf(BmpFileName,"test.bmp");
    WriteBmp(width, height, cTemp[3], cTemp[4], cTemp[5], BmpFileName);

    free(yuv);
    free(rgb);
    return n;
}
//入口 没啥东西
int main()
{
    int i=1;
//    for( i=0; i<260; i++)
        Convert("1280X720_1.yuv", WIDTH, HEIGHT, i);//调用上面的Convert,获取文件的第i帧
    return 0;
}

//写BMP  不必关注
int WriteBmp(int width, int height, unsigned char *R,unsigned char *G,unsigned char *B, char *BmpFileName)
{
    int x=0;
    int y=0;
    int i=0;
    int j=0;
    FILE *fp;
    unsigned char *WRGB;
    unsigned char *WRGB_Start;
    int yu = width*3%4;
    int BytePerLine = 0;

    yu = yu!=0 ? 4-yu : yu;
    BytePerLine = width*3+yu;

    if((fp = fopen(BmpFileName, "wb")) == NULL)
        return 0;
    WRGB = (unsigned char*)malloc(BytePerLine*height+54);
    memset(WRGB, 0, BytePerLine*height+54);
   
    //BMP头
    WRGB[0] = 'B';
    WRGB[1] = 'M';
    *((unsigned int*)(WRGB+2)) = BytePerLine*height+54;
    *((unsigned int*)(WRGB+10)) = 54;
    *((unsigned int*)(WRGB+14)) = 40;
    *((unsigned int*)(WRGB+18)) = width;
    *((unsigned int*)(WRGB+22)) = height;
    *((unsigned short*)(WRGB+26)) = 1;
    *((unsigned short*)(WRGB+28)) = 24;
    *((unsigned short*)(WRGB+34)) = BytePerLine*height;

    WRGB_Start = WRGB + 54;

    for(y=height-1,j=0; y >= 0; y--,j++)
    {
        for(x=0,i=0; x<width; x++)
        {
            WRGB_Start[y*BytePerLine+i++] = B[j*width+x];
            WRGB_Start[y*BytePerLine+i++] = G[j*width+x];
            WRGB_Start[y*BytePerLine+i++] = R[j*width+x];
        }
    }

    fwrite(WRGB, 1, BytePerLine*height+54, fp);
    free(WRGB);
    fclose(fp);
    return 1;
}

/*
 from https://github.com/byhook/ffmpeg4android/blob/master/readme/YUV420P%E3%80%81YUV420SP%E3%80%81NV12%E3%80%81NV21%E5%92%8CRGB%E4%BA%92%E7%9B%B8%E8%BD%AC%E6%8D%A2%E5%B9%B6%E5%AD%98%E5%82%A8%E4%B8%BAJPEG%E4%BB%A5%E5%8F%8APNG%E5%9B%BE%E7%89%87.md
 https://github.com/latelee/yuv2rgb/tree/e5ce9e1d1c9d18495e4149fd8ef8aae4a88f1d59
 
 YUV <——> RGB 转换算法
 http://blog.shenyuanluo.com/ColorConverter.html
 https://github.com/shenyuanluo/SYKit/
*/
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <memory>
#include "SYYuvToRgb.h"
//
//  rgb2jpeg.c
//  libjpegTest
//
//  Created by mawei on 15/10/2.
//  Copyright (c) 2015年 mawei. All rights reserved.
//
#include "rgb2jpeg.h"
//or else ,undefine function
extern "C" {
#include <jpeglib.h>
}
//定义一个压缩对象，这个对象用于处理主要的功能
struct jpeg_compress_struct jpeg;
//用于错误信息
struct jpeg_error_mgr jerr;
int jpeginit(int image_width,int image_height,int quality)
{
    
    //错误输出在绑定
    jpeg.err = jpeg_std_error(&jerr);
    //初始化压缩对象
    jpeg_create_compress(&jpeg);
    //压缩参数设置。具体请到网上找相应的文档吧，参数很多，这里只设置主要的。
    //我设置为一个 24 位的 image_width　X　image_height大小的ＲＧＢ图片
    jpeg.image_width = image_width;
    jpeg.image_height = image_height;
    jpeg.input_components  = 3;
    jpeg.in_color_space = JCS_RGB;
    //参数设置为默认的
    jpeg_set_defaults(&jpeg);
    //还可以设置些其他参数：
    //// 指定亮度及色度质量
    //jpeg.q_scale_factor[0] = jpeg_quality_scaling(100);
    //jpeg.q_scale_factor[1] = jpeg_quality_scaling(100);
    //// 图像采样率，默认为2 * 2
    //jpeg.comp_info[0].v_samp_factor = 1;
    //jpeg.comp_info[0].h_samp_factor = 1;
    //The quality value ranges from 0..100
    jpeg_set_quality(&jpeg, quality, TRUE);
   
    return 0;
}
 
int rgb2jpeg(const char * filename, unsigned char* rgbData)
{
 
    
 
    //定义压缩后的输出，这里输出到一个文件！
    FILE* pFile = fopen( filename,"wb" );
    if( !pFile )
        return 0;
    //绑定输出
    jpeg_stdio_dest(&jpeg, pFile);
    
    //开始压缩。执行这一行数据后，无法再设置参数了！
    jpeg_start_compress(&jpeg, TRUE);
    
    JSAMPROW row_pointer[1];
    //从上到下，设置图片中每一行的像素值
    for( int i=0;i<jpeg.image_height;i++ )
    {
        row_pointer[0] = rgbData+i*jpeg.image_width*3;
        jpeg_write_scanlines( &jpeg,row_pointer,1 );
    }
    //结束压缩
    jpeg_finish_compress(&jpeg);
 
    
    fclose( pFile );
    pFile = NULL;
    return 0;
    
}
int jpeguninit()
{
    //清空对象
    jpeg_destroy_compress(&jpeg);
    return 0;
}
void YUV420P_TO_RGB24(unsigned char *data, unsigned char *rgb, int width, int height) {
    int index = 0;
    unsigned char *ybase = data;
    unsigned char *ubase = &data[width * height];
    unsigned char *vbase = &data[width * height * 5 / 4];
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            //YYYYYYYYUUVV
            u_char Y = ybase[x + y * width];
            u_char U = ubase[y / 2 * width / 2 + (x / 2)];
            u_char V = vbase[y / 2 * width / 2 + (x / 2)];


            rgb[index++] = Y + 1.402 * (V - 128); //R
            rgb[index++] = Y - 0.34413 * (U - 128) - 0.71414 * (V - 128); //G
            rgb[index++] = Y + 1.772 * (U - 128); //B
        }
    }
}
bool YV12_to_RGB24(unsigned char* pYV12, unsigned char* pRGB24, int iWidth, int iHeight)
{
 if(!pYV12 || !pRGB24)
    return false;
	
 const long nYLen = long(iHeight * iWidth);
 const int nHfWidth = (iWidth>>1);
 
 if(nYLen < 1 || nHfWidth < 1) 
    return false;
	
 unsigned char* yData = pYV12;
 unsigned char* vData = &yData[nYLen];
 unsigned char* uData = &vData[nYLen>>2];
 if(!uData || !vData)
    return false;
 
 int rgb[3];
 int i, j, m, n, x, y;
 m = -iWidth;
 n = -nHfWidth;
 for(y = 0; y < iHeight; y++)
 {
    m += iWidth;
	
    if(!(y % 2))
		n += nHfWidth;
		
    for(x=0; x < iWidth; x++)
    {
		i = m + x;
		j = n + (x>>1);
		rgb[2] = int(yData[i] + 1.370705 * (vData[j] - 128)); // r分量值
		rgb[1] = int(yData[i] - 0.698001 * (uData[j] - 128)  - 0.703125 * (vData[j] - 128)); // g分量值
		rgb[0] = int(yData[i] + 1.732446 * (uData[j] - 128)); // b分量值
		j = nYLen - iWidth - m + x;
		i = (j<<1) + j;
		for(j=0; j<3; j++)
		{
			if(rgb[j]>=0 && rgb[j]<=255)
				pRGB24[i + j] = rgb[j];
			else
				pRGB24[i + j] = (rgb[j] < 0) ? 0 : 255;
		}
    }
 }
 
 return true;
}
/*
https://segmentfault.com/a/1190000016443536
https://github.com/zhuzhzh/myrgb2yuv420p
*/
void Yuv420p2Rgb888(uint8_t *destination, uint8_t *yuv, size_t width, size_t height)
{
    size_t image_size = width * height;
    size_t upos = image_size;
    size_t vpos = upos + upos / 4;
    size_t i = 0;

    for( size_t line = 0; line < height; ++line )
    {
		for( size_t col = 0; col < width; col += 1 )
		{
			uint8_t y = yuv[line*width+col];
			uint8_t u = yuv[(line/2)*(width/2)+(col/2)+image_size];
			uint8_t v = yuv[(line/2)*(width/2)+(col/2)+image_size+(image_size/4)];

			int16_t C = y-16;
			int16_t D = u-128;
			int16_t E = v-128;

			int16_t rt =  (int16_t)((298*C+408*E+128)>>8);
			int16_t gt =  (int16_t)((298*C-100*D-208*E+128)>>8);
			int16_t bt =  (int16_t)((298*C+516*D+128)>>8);

			destination[i++] = rt>255?255:rt<0?0:rt;
			destination[i++] = gt>255?255:gt<0?0:gt;
			destination[i++] = bt>255?255:bt<0?0:bt;

		}
    }
}
const char *yuv_in="1280X720_1.yuv";
const char *rgb_out="rgb.out";
const char *jpeg_out="1.jpeg";
int width=1280;
int height=720;
int main(){
    std::ifstream fin(yuv_in, std::ios::binary);
    int read_len=width*height*3/2;
    std::unique_ptr<unsigned char []> yuv_buf(new unsigned char [read_len]);
    fin.read((char*)yuv_buf.get(), sizeof(char) * 256);
    fin.close();
    int write_size=width*height*3;
    std::unique_ptr<unsigned char []> rgb_buf(new unsigned char [write_size]);
    //YV12_to_RGB24((unsigned char*)yuv_buf.get(),(unsigned char*)rgb_buf.get(),width,height);
    SYYuvToRgb syyuv3rgb;
    syyuv3rgb.SY_I420ToRgb24((unsigned char*)yuv_buf.get(),width,height,(unsigned char*)rgb_buf.get());
    //Yuv420p2Rgb888((uint8_t*)rgb_buf.get(),(uint8_t*)yuv_buf.get(),width,height);
    std::ofstream out;
    out.open(rgb_out,std::ofstream::binary);
    out.write(reinterpret_cast<const char*>(rgb_buf.get()),write_size);
    out.close();
    
    jpeginit(width,height,80);
    rgb2jpeg(jpeg_out,(unsigned char*)rgb_buf.get());
    return 0;
}

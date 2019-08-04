//
//  rgb2jpeg.h
//  libjpegTest
//
//  Created by mawei on 15/10/2.
//  Copyright (c) 2015年 mawei. All rights reserved.
//
 
#ifndef __libjpegTest__rgb2jpeg__
#define __libjpegTest__rgb2jpeg__
 
#include <stdio.h>
 
 
 
int jpeginit(int image_width,int image_height,int quality);
 
 
 
int rgb2jpeg(char * filename, unsigned char* rgbData);
 
int jpeguninit();
#endif /* defined(__libjpegTest__rgb2jpeg__) */
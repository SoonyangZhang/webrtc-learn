#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <string>
#include "psnr.h"
int main(int n, char *cl[])
{
  FILE *f1=0, *f2=0,*f3=0;
  int i, x, y, yuv, inc = 1, Y, F;
  unsigned char *b1, *b2;
  // after goto, no declaration
  int file_len=0, frames_in_f1,frames_in_f;
  double yssim,ypsnr;
  if (n != 7 && n != 8) {
    puts("psnr x y <YUV format> <src.yuv> <dst.yuv> <result.txt> [multiplex]");
    puts("  x\t\tframe width");
    puts("  y\t\tframe height");
    puts("  YUV format\t420, 422, etc.");
    puts("  src.yuv\tsource video");
    puts("  dst.yuv\tdistorted video");
    puts("  result.txt");
    puts("  [multiplex]\toptional");
    return EXIT_FAILURE;
  }

  if ((f1 = fopen(cl[4], "rb")) == 0) goto A;
  if ((f2 = fopen(cl[5], "rb")) == 0) goto B;
  if (!(x = strtoul(cl[1], 0, 10)) ||
      !(y = strtoul(cl[2], 0, 10))) goto C; 
  if ((yuv = strtoul(cl[3], 0, 10)) > 444) goto D;
  if ((f3 = fopen(cl[6], "w+")) == 0) goto A;
  if (cl[7] && !strcmp(cl[7], "multiplex")) inc = 2;

  Y = x * y;
  switch (yuv) {
    case 400: F = Y; break;
    case 422: F = Y * 2; break;
    case 444: F = Y * 3; break;
    default :
    case 420: F = Y * 3 / 2; break;
  }
  fseek (f1 , 0 , SEEK_END);
  file_len=ftell (f1);
  rewind(f1);
  frames_in_f1=file_len/F;
  fseek (f2 , 0 , SEEK_END);
  file_len=ftell (f2);
  rewind(f2);
  frames_in_f=file_len/F;
  if(frames_in_f1<frames_in_f){
	  frames_in_f=frames_in_f1;
  }
  if (!(b1 =(unsigned char*)malloc(F))) goto E;
  if (!(b2 =(unsigned char*)malloc(F))) goto E;
  for (i=0;i<frames_in_f;i++) {
    if (1 != fread(b1, F, 1, f1) || 1 != fread(b2, F, 1, f2)) break;
    yssim=x264_pixel_ssim_wxh(b1, x, b2, x, x, y);
	ypsnr=calculate_psnr(b1,b2,inc,inc == 1 ? Y : F);
    std::string line=std::to_string(i)+std::string("\t")+
    				 std::to_string(yssim)+std::string("\t")+
					 std::to_string(ypsnr)+std::string("\n");
    fwrite(line.c_str(),1,line.size(),f3);
  }
  fclose(f1);
  fclose(f2);
  fclose(f3);
  free(b1);
  free(b2);
  //fprintf(stderr, "%s:\t%d frames (CPU: %lu s) mean: %.2f stdv: %.2f\n",
  //  ssim ? "ssim" : "psnr", N, (unsigned long) ((clock() - t) / CLOCKS_PER_SEC), mean, stdv);

  return 0;

A: fprintf(stderr, " Error opening source video file.\n"); goto X;
B: fprintf(stderr, " Error opening decoded video file.\n"); goto X;
C: fprintf(stderr, " Invalid width or height.\n"); goto X;
D: fprintf(stderr, " Invalid YUV format.\n"); goto X;
E: fprintf(stderr, " Not enough memory.\n");
X: return EXIT_FAILURE;
}

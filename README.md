# webrtc-learn
learn write code from webrtc project  
the webrtc lib ia from   
https://sourcey.com/precompiled-webrtc-libraries   
version webrtc-22215-ab42706-linux-x64.tar.gz  

bmp2jpep  https://github.com/ValleyXu/Conversion-Between-JPEG-and-BMP  
SYConverter https://github.com/shenyuanluo/SYKit  

other tools for possible usage:  
https://github.com/IENT/YUView  
x264 install  
```
https://www.cnblogs.com/ziyu-trip/p/6783165.html
rror while loading shared libraries: libx264.so.2:   
cannot open shared object file: No such file or directory  
su
echo "/usr/local/lib" >> /etc/ld.so.conf
ldconfig

https://stackoverflow.com/questions/11838456/undefined-reference-to-x264-encoder-open-125/17174658  
apt-get remove x264
LD_LIBRARY_PATH=/path/to/my/compiled/x264/library
```
ffmpeg  
```
https://blog.csdn.net/a805995924/article/details/39435261
https://gist.github.com/teocci/f7a438013a0197a91446ee86de41faee
```
ffmepg yuv 
```
ffplay -f rawvideo -pixel_format rgb24 -s 480*480 texture.rgb
```
YUV和RGB详解: https://segmentfault.com/a/1190000016443536  

export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH  

ffplay h264 raw:  
https://github.com/SoonyangZhang/base/tree/master/video_tool    
ffplay -stats -f h264 1280x720.h264  


h264 tool:  
https://github.com/latelee/H264BSAnalyzer 

x264 slice type
```
* Slice type */
#define X264_TYPE_AUTO          0x0000  /* Let x264 choose the right type */
#define X264_TYPE_IDR           0x0001
#define X264_TYPE_I             0x0002
#define X264_TYPE_P             0x0003
#define X264_TYPE_BREF          0x0004  /* Non-disposable B-frame */
#define X264_TYPE_B             0x0005
```
slice type  
http://gentlelogic.blogspot.com/2011/11/exploring-h264-part-2-h264-bitstream.html  
指数哥伦布编码：
https://www.jianshu.com/p/f4f3b0764cec  
x264  preset
https://blog.csdn.net/NB_vol_1/article/details/78363559  

I found that, The operation to refer frame instead of copy can reduce frame queue delay and reduce frame dropping.  
```
//videocodec.h  
//old version
struct FrameTs{
  FrameTs(webrtc::VideoFrame *f,uint32_t ts):frame(f),enqueTs(ts){}	
  webrtc::VideoFrame *frame;
  uint32_t enqueTs;
};
//videocodec.cc   
webrtc::VideoFrame *copy=new webrtc::VideoFrame(frame);  
f=temp.frame;  
//improve version  
struct FrameTs{  
  FrameTs(const webrtc::VideoFrame &f,uint32_t ts):frame(f),enqueTs(ts){}  
  FrameTs(const FrameTs&)=default;  //for deque front  
  webrtc::VideoFrame frame;  
  uint32_t enqueTs;  
};

```




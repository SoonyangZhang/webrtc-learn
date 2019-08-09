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
ffplay -stats -f h264 ds2.h264  


h264 tool:  
https://github.com/latelee/H264BSAnalyzer 


dropper is used to test the effect when some of P frames are dropped.  
dropper should be used with h264-parser to get the h264 raw stream nalu info(1280x720_info.txt).  
h264-patser  https://github.com/SoonyangZhang/h264-parser  
in xx.h264(test h264 rar bitstream)  out out.h264.  

Conclusion:  
It seems there is no obvious visual effect when the frames (drop_table[]={6,7,8,9}) between two I frames are lost.  
But lost some part I frame will do (drop_table[]={5,6,7,8}),since 4 and 5 are two IDR frames.  



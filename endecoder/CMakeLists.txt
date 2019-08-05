include_directories(${CMAKE_SOURCE_DIR}/encoder)
set(endecoder_FILES
echo_h264_encoder.cc
#echo_h264_decoder.cc
H264Decoder.cc
videocodec.cc
)
add_library(h264endecoder STATIC ${endecoder_FILES})

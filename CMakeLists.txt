PROJECT(project)
cmake_minimum_required(VERSION 2.6)
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_definitions(-DWEBRTC_WIN)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_definitions(-DWEBRTC_POSIX -DWEBRTC_LINUX)
endif()
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall  -O2")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O2")
set(CMAKE_CXX_FLAGS "-fPIC")  
add_definitions(-D__STDC_FORMAT_MACROS)
# to handle undefined reference to `rtc::Thread::Clear(rtc::MessageHandler*, unsigned int, std::__cxx11::list
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
add_definitions(-Dlinux_plat)
#add_definitions(-DNDEBUG)
#add_definitions(-std=c++11  )#-fno-rtti
#set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(${CMAKE_SOURCE_DIR}/)
include_directories(${CMAKE_SOURCE_DIR}/base)
include_directories(${CMAKE_SOURCE_DIR}/bmp2jpeg)
include_directories(${CMAKE_SOURCE_DIR}/endecoder)
include_directories(${CMAKE_SOURCE_DIR}/h264parser)
include_directories(${CMAKE_SOURCE_DIR}/logging)
include_directories(${CMAKE_SOURCE_DIR}/packetizer)
include_directories(${CMAKE_SOURCE_DIR}/record)
include_directories(${CMAKE_SOURCE_DIR}/stream)
include_directories(${CMAKE_SOURCE_DIR}/video_capture)
include_directories(${CMAKE_SOURCE_DIR}/tools)

include_directories(${CMAKE_SOURCE_DIR}/../webrtc/include/)
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/../webrtc/lib/x64/Release)

include_directories(/usr/local/include)
LINK_DIRECTORIES(/usr/local/lib)

add_subdirectory(endecoder)

set(zsy_LIB
record/yuv_record.cc
record/h264_record.cc
)
add_library(zsy STATIC ${zsy_LIB})

set(psnr_LIB
tools/psnr.c
)
add_library(psnr STATIC ${psnr_LIB})

set(bmp2jp_LIB
bmp2jpeg/bmp.cc
bmp2jpeg/dct.cc
bmp2jpeg/encode.cc
bmp2jpeg/huffencode.cc
bmp2jpeg/io.cc
bmp2jpeg/jpeg.cc
)
add_library(bmp2jp STATIC ${bmp2jp_LIB})

set(base_LIB
base/base_time.cc
base/callback.cc
base/copy_on_write_buffer.cc
base/my_thread.cc
base/task_queue.cc
)
add_library(base STATIC ${base_LIB})

set(nonrtp_LIB
packetizer/non_rtp.cc
packetizer/non_rtp_h264.cc
packetizer/non_rtp_packet_to_send.cc
packetizer/non_rtp_packet.cc
h264parser/h264_parser.cc
stream/rtc_send_stream.cc
stream/rtc_recv_stream.cc
)
add_library(nonrtp STATIC ${nonrtp_LIB})


set(EXECUTABLE_NAME "video_capture")
add_executable(${EXECUTABLE_NAME} main.cc)
target_link_libraries(${EXECUTABLE_NAME} nonrtp  h264endecoder  zsy base  psnr webrtc_full  x264 avcodec avutil swscale   pthread dl m)
set(EXECUTABLE_NAME "fun_test")
add_executable(${EXECUTABLE_NAME} fun_test.cc )
target_link_libraries(${EXECUTABLE_NAME} zsy base  psnr webrtc_full pthread)

set(EXECUTABLE_NAME "yuv2bmp")
add_executable(${EXECUTABLE_NAME} yuv2bmp.cc )

set(EXECUTABLE_NAME "bmp2jpeg")
add_executable(${EXECUTABLE_NAME} bmp2jpeg/bmp2jpeg.cc )
target_link_libraries(${EXECUTABLE_NAME} bmp2jp )

#./t_psnr 640 480 420 videoraw.yuv file.yuv ssim > ref_ssim.txt
set(EXECUTABLE_NAME "t_psnr")
add_executable(${EXECUTABLE_NAME} tools/t_psnr.cc)
target_link_libraries(${EXECUTABLE_NAME} psnr m)

set(EXECUTABLE_NAME "t_ffmpeg")
add_executable(${EXECUTABLE_NAME} ${CMAKE_SOURCE_DIR}/endecoder/ffmpeg_test.cc)
target_link_libraries(${EXECUTABLE_NAME} h264endecoder  x264 avcodec avutil swscale pthread dl m)
# h264endecoder  x264 the link order matters.
#su
#echo "/usr/local/lib" >> /etc/ld.so.conf
#ldconfig
set(EXECUTABLE_NAME "t_task")
add_executable(${EXECUTABLE_NAME} ${CMAKE_SOURCE_DIR}/base/thread_test.cc)
target_link_libraries(${EXECUTABLE_NAME} base pthread)
#https://github.com/SoonyangZhang/base/tree/master/video_tool  


set(EXECUTABLE_NAME "t_packet")
add_executable(${EXECUTABLE_NAME} ${CMAKE_SOURCE_DIR}/packetizer/packet_test.cc)
target_link_libraries(${EXECUTABLE_NAME} base nonrtp)

set(EXECUTABLE_NAME "t_parser")
add_executable(${EXECUTABLE_NAME} ${CMAKE_SOURCE_DIR}/h264parser/parser_test.cc)
target_link_libraries(${EXECUTABLE_NAME} base nonrtp)

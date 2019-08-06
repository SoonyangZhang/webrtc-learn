PROJECT(project)
cmake_minimum_required(VERSION 2.6)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall  -O2")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O2")
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_definitions(-DWEBRTC_WIN)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_definitions(-DWEBRTC_POSIX -DWEBRTC_LINUX)
endif()
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall  -O2")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O2")
set(CMAKE_CXX_FLAGS "-fPIC")  
set(CMAKE_C_FLAGS "-fPIC") 
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
include_directories(${CMAKE_SOURCE_DIR}/fun_test)
include_directories(${CMAKE_SOURCE_DIR}/fake)
include_directories(${CMAKE_SOURCE_DIR}/bmp2jpeg)
include_directories(${CMAKE_SOURCE_DIR}/video_capture)
include_directories(${CMAKE_SOURCE_DIR}/endecoder)

include_directories(${CMAKE_SOURCE_DIR}/../webrtc/include/)
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/../webrtc/lib/x64/Release)

include_directories(/usr/local/include)
LINK_DIRECTORIES(/usr/local/lib)

add_subdirectory(endecoder)

set(zsy_LIB
fun_test/yuvrecord.cc
)
add_library(zsy STATIC ${zsy_LIB})

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
base/my_thread.cc
)
add_library(base STATIC ${base_LIB})

set(fake_LIB
fake/fake_video.cc
)
add_library(fake STATIC ${fake_LIB})

set(EXECUTABLE_NAME "video_capture")
add_executable(${EXECUTABLE_NAME} main.cc)
target_link_libraries(${EXECUTABLE_NAME}   h264endecoder base zsy webrtc_full  x264 avcodec avutil swscale pthread dl m)
set(EXECUTABLE_NAME "fun_test")
add_executable(${EXECUTABLE_NAME} fun_test.cc )
target_link_libraries(${EXECUTABLE_NAME}  zsy webrtc_full pthread)

set(EXECUTABLE_NAME "yuv2bmp")
add_executable(${EXECUTABLE_NAME} yuv2bmp.cc )

set(EXECUTABLE_NAME "bmp2jpeg")
add_executable(${EXECUTABLE_NAME} bmp2jpeg/bmp2jpeg.cc )
target_link_libraries(${EXECUTABLE_NAME} bmp2jp )

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
set(EXECUTABLE_NAME "t_fake")
add_executable(${EXECUTABLE_NAME} ${CMAKE_SOURCE_DIR}/fake/fake_test.cc)
target_link_libraries(${EXECUTABLE_NAME} fake webrtc_full pthread)

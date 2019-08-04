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
#add_definitions(-DNDEBUG)
#add_definitions(-std=c++11  )#-fno-rtti
#set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(${CMAKE_SOURCE_DIR}/)
include_directories(${CMAKE_SOURCE_DIR}/fun_test)
include_directories(${CMAKE_SOURCE_DIR}/webrtc/include/)
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/webrtc/lib/x64/Release)

set(zsy_LIB
fun_test/yuv2jpeg.cc
)

add_library(zsy STATIC ${zsy_LIB})
set(SY_LIB
SYYuvToRgb.cc
)
add_library(SY STATIC ${SY_LIB})
set(EXECUTABLE_NAME "video_capture")
add_executable(${EXECUTABLE_NAME} main.cc)
target_link_libraries(${EXECUTABLE_NAME}  zsy webrtc_full pthread)
set(EXECUTABLE_NAME "fun_test")
add_executable(${EXECUTABLE_NAME} fun_test.cc )
target_link_libraries(${EXECUTABLE_NAME}  zsy webrtc_full pthread)

set(EXECUTABLE_NAME "yuv2rgb")
include_directories(${CMAKE_SOURCE_DIR}/jpeg-6b/)
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/jpeg-6b/.libs/)
add_executable(${EXECUTABLE_NAME} yuv2rgb.cc )
target_link_libraries(${EXECUTABLE_NAME} SY jpeg)


set(EXECUTABLE_NAME "yuv2bmp")
add_executable(${EXECUTABLE_NAME} yuv2bmp.cc )


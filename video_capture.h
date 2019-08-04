#pragma once
#include "video_bridge.h"
namespace zsy
{
class VideoCapture{
public:
 VideoCapture() : number_of_devices_(0) {}
 void SetUp() {
   device_info_.reset(webrtc::VideoCaptureFactory::CreateDeviceInfo());
   assert(device_info_.get());
   number_of_devices_ = device_info_->NumberOfDevices();
   assert(number_of_devices_>0u);
 }
 void OpenVideoCaptureDevice()
 {
	    char device_name[256];
	    char unique_name[256];
	    capture_module_=webrtc::VideoCaptureFactory::Create(unique_name);
	    capture_module_->RegisterCaptureDataCallback(&capture_callback_);
 }
 void GetCapability(webrtc::VideoCaptureCapability &capability)
 {
	 device_info_->GetCapability(capture_module_->CurrentDeviceName(), 0, capability);
 }
 void StartCapture(webrtc::VideoCaptureCapability capability)
 {
	 capture_module_->StartCapture(capability);
 }
 void StopCapture()
{
   capture_module_->StopCapture();
}
 bool AddSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink){
	 return capture_callback_.AddSink(sink);
 }
 bool RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink){
	 return capture_callback_.RemoveSink(sink);
 }
private:
 uint32_t number_of_devices_;
 std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> device_info_;
 VideoCaptureCallback capture_callback_;
 rtc::scoped_refptr<webrtc::VideoCaptureModule> capture_module_;
};
}


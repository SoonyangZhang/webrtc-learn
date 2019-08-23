#include "stream/rtc_send_stream.h"
#include "stream/rtc_recv_stream.h"
#include "base/array_view.h"
#include "base/base_time.h"
#include "h264parser/h264_parser.h"
#include "packetizer/non_rtp_packet_to_send.h"
#include "packetizer/non_rtp.h"
#include "packetizer/non_rtp_h264.h"
#include "logging.h"
namespace zsy{
enum FrameType:uint32_t{
	X264_FRAME_AUTO=0x0000,
	X264_FRAME_IDR=0x0001,
	X264_FRAME_I=0x0002,
	X264_FRAME_P=0x0003,
	X264_FRAME_BREF=0x0004,
	X264_FRAME_B=0x0005,
};
RTCSendStream::RTCSendStream(){}
RTCSendStream::~RTCSendStream(){}
void RTCSendStream::StartSender(){
	running_=true;
	Start();
}
void RTCSendStream::StopSender(){
	running_=false;
	Stop();
}
void RTCSendStream::Run(){
	while(running_){
		PakcetizeImage();
		if(HasDataToSend()){
			SendPacket();
		}
	}
}
void RTCSendStream::OnEncodedImageCallBack(EncodeImage &image){
	if(frame_id_>5){
		return;
	}
	if(!running_){
		return;
	}
	LockScope crit(&que_lock_);
	images_.push_back(image);
	image_count_++;
}
bool RTCSendStream::HasDataToSend() const{
	return (offset_>0)&&(offset_>wait_send_offset_);
}
void RTCSendStream::PakcetizeImage(){
	std::list<EncodeImage> temp;
	if(image_count_>0){
		LockScope crit(&que_lock_);
		while(!images_.empty()){
			EncodeImage image=images_.front();
			images_.pop_front();
			temp.push_back(image);
			image_count_--;
		}
	}
	while(!temp.empty()){
		EncodeImage image=temp.front();
		temp.pop_front();
		uint32_t capture_ts=image.CaptureTs();
		uint32_t ft=image.FrameType();
        bool key_frame=false;
        uint8_t payload_type=PT_H264_DELTA;
        if(ft==X264_TYPE_IDR||ft==X264_TYPE_I){
        	key_frame=true;
        	payload_type=PT_H264_KEY;
        }
		std::list<H264NALU> nalus;
	    H264Parser parser;
	    const uint8_t *start=image.data();
	    size_t frame_len=image.size();
	    DLOG(INFO)<<"frame len "<<frame_len;
	    parser.SetStream(start,frame_len);
	    while(true){
	    	H264NALU nalu;
	        if(parser.AdvanceToNextNALU(&nalu)==H264Parser::kOk){
	        	nalus.push_back(nalu);
	        }else{
	            break;
	        }
	    }
	    DCHECK_GT(nalus.size(),0);
	    NonRtpFragmentHead header(nalus.size());
	    size_t i=0;
	    for(auto it=nalus.begin();it!=nalus.end();it++){
	    	header.fragmentLength[i]=it->size;
	    	header.fragmentOffset[i]=it->data-start;
	    	i++;
	    }
	    if(i>0){
	    	frame_id_++;
	        ArrayView<const uint8_t> payload(start,frame_len);
	        PayloadSizeLimits limits;
	        NonRtpPacketizerH264  h264_packetizer(payload,limits,H264PacketizationMode::NonInterleaved,header);
	        NonRtpPacketToSend *packet=nullptr;
	        size_t num_packets=h264_packetizer.num_packet_left();
	        bool ret=false;
	        uint32_t now=GetMilliSeconds();
	        for(i=0;i<num_packets;i++){
	        	packet=new NonRtpPacketToSend();
	        	packet->ReserveHeaderSpace();
	        	packet->SetPayloadType(payload_type);
	        	packet->SetTimestamp(capture_ts);
	        	packet->SetGroupId(group_id_);
	        	packet->SetPacketsPerFrame(num_packets);
	        	ret=h264_packetizer.NextPacket(packet);
	        	if(ret){
	        		size_t packet_size=packet->size();
	        		SendPacketInfo info(offset_,packet_size,now,(uint8_t)ft,0);
	        		info.buffer=packet;
	        		packets_.insert(std::make_pair(offset_,info));
	        		offset_+=packet_size;
	        	}else{
	        		delete packet;
	        	}
	        }
	        DCHECK_EQ(h264_packetizer.num_packet_left(),0);
	    }
		if((!first_frame_)&&key_frame){
			group_id_++;
		}
		if(first_frame_){
			first_frame_=false;
		}
	}
}
void RTCSendStream::SendPacket(){
    if(frame_id_>5){
    	return;
    }
	while(!packets_.empty()){
		auto it=packets_.begin();
		uint64_t offset=it->first;
		SendPacketInfo info=it->second;
		NonRtpPacketToSend *packet=info.buffer;
		size_t length=info.length;
		if(target_){
			target_->OnNewData(offset,packet->data(),length);
		}
		wait_send_offset_+=length;
		info.DeleteBuffer();
		packets_.erase(it);
	}

}
}

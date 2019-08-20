#include "array_view.h"
#include "rtc_recv_stream.h"
#include "non_rtp_packet.h"
#include "logging.h"
namespace zsy{
const char start3[]={0x00,0x00,0x01};
const char start4[]={0x00,0x00,0x00,0x01};
const uint8_t kTypeMask = 0x1F;
RTCRecvStream::RTCRecvStream(std::string &s){
	f_out_.open(s.c_str(),std::fstream::out);
}
RTCRecvStream::~RTCRecvStream(){
	if(f_out_.is_open()){
		f_out_.close();
	}
}
void RTCRecvStream::OnNewData(const uint8_t *data,size_t size){
	NonRtpPacket parser;
	bool ret=false;
	ret=parser.Parse(data,size);
	if(ret){
		uint32_t frame_id=parser.FrameId();
		uint32_t group_id=parser.GroupId();
		ArrayView<const uint8_t> payload=parser.payload();
		size_t payload_size=payload.size();
		uint32_t type=payload[0]&kTypeMask;
		if(f_out_.is_open()){
			char line [512];
			memset(line,0,512);
			sprintf (line, "%u %u %u %u",type,frame_id,
					group_id,payload_size);
			f_out_<<line<<std::endl;
		}
	}else{
		DLOG(INFO)<<"packet parser error";
	}
}
}

#pragma once
#include <stdint.h>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include "base/optional.h"
#include "base/wrapper.h"
#include "base/array_view.h"
#include "base/byte_rw.h"
#include "packetizer/h264_common.h"
#include "packetizer/non_rtp_packet.h"
#include "packetizer/non_rtp_h264.h"
#include "packetizer/non_rtp.h"
namespace zsy{
using GroupNumberUnwrapper = Unwrapper<uint8_t>;
class RecvFrame;
class RTCRecvStream{
public:
class Delegate{
public:
	virtual void OnFrameAvailable(const uint8_t *data,size_t size)=0;
	virtual ~Delegate(){}
};
	RTCRecvStream(std::string &s);
	~RTCRecvStream();
	void OnNewData(uint64_t offset,const uint8_t *data,size_t size);
	void CheckDeliverable();
private:
	std::map<uint32_t,RecvFrame*> frames_;
	GroupNumberUnwrapper group_id_;
	std::fstream f_out_;
	uint64_t last_comsumed_offset_{0};
};
class RecvNalu{
public:
RecvNalu(){}
~RecvNalu(){}
RecvNalu(RecvNalu&&r){
	*this=std::move(r);
}
RecvNalu& operator=(RecvNalu&&r);
RecvNalu(const RecvNalu &)=default;
RecvNalu& operator=(const RecvNalu &)=default;
void OnNewPacket(const uint8_t *data,int length);
void SetPacketizerMode(uint8_t mode);
uint8_t GetPacketizerMode() const{
	return packetizer_mode_;
}
uint8_t packetizer_mode_{0};
uint8_t nalu_type_;
uint8_t start_code_len_{0};
uint32_t timestamp_;
int total_length_{0};
std::vector<const uint8_t *> payloads_;
std::vector<int> lengths_;

};
class RecvFrame{
public:
	struct OffsetAndLength{
	OffsetAndLength(uint64_t o,uint32_t l):
	offset(o),length(l){}
	uint64_t offset;
	uint32_t length;
	bool operator<(const OffsetAndLength&a) const{
	    return offset<a.offset;
	}
	};
	RecvFrame(){}
	~RecvFrame();
	void OnNewPacket(uint64_t offset,uint32_t len,NonRtpPacket *packet);
	uint8_t Received() const{
		return received_;
	}
	uint8_t PacketsPerFrame() const{
		return packets_of_frame_;
	}
	uint32_t GetNaluSize() const{
		return nalus_.size();
	}
	uint8_t GetStatCodeLenOfNalu(size_t i) const{
		return nalus_[i].start_code_len_;
	}
	bool IsFullFrame() const;
	uint8_t FrameType() const{
		return frame_type_;
	}
	uint64_t GetOffsetAfterConsume() const;
	// data size after parse
	uint32_t GetDelivableSize();
    bool ParserStapAToRecvNalu(NonRtpPacket *packet,std::vector<RecvNalu>&nalus);
    bool ParserFuAToRecvNalu(NonRtpPacket *packet,std::vector<RecvNalu>&nalus);
    bool ParserPacketToRecvNalu(NonRtpPacket *packet,std::vector<RecvNalu>&nalus);
private:
	uint8_t frame_type_;
	uint8_t received_{0};
	uint8_t packets_of_frame_{0};
	uint32_t capture_ts_;
	std::map<OffsetAndLength,NonRtpPacket*> packets_;
	std::vector<RecvNalu> nalus_;
};
}

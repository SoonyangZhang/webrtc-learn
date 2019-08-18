#include "non_rtp_packet.h"
#include "logging.h"
#include "byte_rw.h"
namespace zsy{
/*
 * PT(1)   ts(4)  ty(1);
 * ty: single,FU-A,STRAP-A
 * since I transmit the video packer on quic,
 * the header of RTP is thus overlooker
 * quic protocol I copy from google:https://github.com/SoonyangZhang/DrainQueueCongestion
 */
namespace{
	const size_t kFixedHeaderSize=5;
	const size_t kDefaultPacketSize = 1500;
}
NonRtpPacket::NonRtpPacket():NonRtpPacket(kDefaultPacketSize){

}
NonRtpPacket::NonRtpPacket(size_t capacity):buffer_(capacity){
	//ReserveHeaderSpace(kFixedHeaderSize);
}
NonRtpPacket::NonRtpPacket(const NonRtpPacket&)=default;
void NonRtpPacket::ReserveHeaderSpace(){
	payload_offset_=kFixedHeaderSize;
	DCHECK_GE(buffer_.capacity(),payload_offset_);
}
void NonRtpPacket::SetPayloadType(uint8_t payload_type){
	payload_type_=payload_type;
	WriteAt(0,(data()[0]&0x80)|payload_type);
}
void NonRtpPacket::SetTimestamp(uint32_t timestamp){
	time_stamp_=timestamp;
	ByteWriter<uint8_t>::WriteBigEndian(WriteAt(1),time_stamp_);
}
void NonRtpPacket::Clear(){
    marker_=false;
    payload_type_=0;
    time_stamp_=0;
	payload_offset_=kFixedHeaderSize;
	payload_size_=0;
}
uint8_t* NonRtpPacket::AllocatePayload(size_t size_bytes){
	SetPayloadSize(0);
	return SetPayloadSize(size_bytes);
}
uint8_t* NonRtpPacket::SetPayloadSize(size_t size_bytes){
	if(payload_offset_+size_bytes>capacity()){
		DLOG(ERROR)<<"non enough space";
		return nullptr;
	}
	  payload_size_ = size_bytes;
	  buffer_.SetSize(payload_offset_ + payload_size_);
	  return WriteAt(payload_offset_);
}
bool NonRtpPacket::Parse(const uint8_t *buffer,size_t buffer_size){
	if(!ParseBuffer(buffer,buffer_size)){
        Clear();
        return false;
	}
	buffer_.SetData(buffer,buffer_size);
	return true;
}
bool NonRtpPacket::Parse(ArrayView<const uint8_t> packet){
	return Parse(packet.data(),packet.size());
}
bool NonRtpPacket::ParseBuffer(const uint8_t *buffer,size_t size){
	if(size<kFixedHeaderSize){
		return false;
	}
	marker_=buffer[0]&0x08;
	payload_type_=buffer[0]&0x7f;
	time_stamp_=ByteReader<uint32_t>::ReadBigEndian((uint8_t*)&buffer[1]);
	payload_offset_=kFixedHeaderSize;
	payload_size_=size-kFixedHeaderSize;
	return true;
}
}

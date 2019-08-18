#pragma once
#include <stdint.h>
#include "copy_on_write_buffer.h"
namespace zsy{
class NonRtpPacket{
public:
	NonRtpPacket();
	NonRtpPacket(size_t capacity);
	NonRtpPacket(const NonRtpPacket&);
	NonRtpPacket& operator=(const NonRtpPacket&) = default;
	void ReserveHeaderSpace();
	void SetPayloadType(uint8_t paylload_type);
	void SetTimestamp(uint32_t timestamp);
	void Clear();
	uint8_t *WriteAt(size_t offset){
		return buffer_.data()+offset;}
	void WriteAt(size_t offset,uint8_t byte){
		buffer_.data()[offset]=byte;
	}
	const uint8_t* ReadAt(size_t offset) const{
		return buffer_.data()+offset;
	}
	uint8_t* AllocatePayload(size_t size_bytes);
	uint8_t* SetPayloadSize(size_t size_bytes);
	size_t header_size() const{ return payload_offset_;}
	size_t payload_size() const { return payload_size_;}
	uint8_t PayloadType() const { return payload_type_;}
	const uint8_t* data()const {
		return buffer_.data();
	}
	size_t size() const {return payload_offset_+payload_size_;}
	size_t capacity() const {
		return buffer_.capacity();
	}
	size_t FreeCapacity() const{ return capacity()-size();}
	zsy::ArrayView<const uint8_t> payload() const {
	  return zsy::MakeArrayView(data() + payload_offset_, payload_size_);
	}
	bool Parse(const uint8_t *buffer,size_t buffer_size);
	bool Parse(ArrayView<const uint8_t> packet);
private:
	bool ParseBuffer(const uint8_t *buffer,size_t size);
	bool marker_;
	uint8_t payload_type_;
	uint32_t time_stamp_;
	CopyOnWriteBuffer buffer_;
	size_t payload_offset_{0};
	size_t payload_size_{0};
};
}

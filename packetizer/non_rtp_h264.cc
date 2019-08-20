#include <memory.h>
#include "packetizer/non_rtp_h264.h"
#include "packetizer/h264_common.h"
#include "base/byte_rw.h"
#include "logging/logging.h"
namespace zsy{
namespace{
static const size_t kNalHeaderSize = 1;
static const size_t kFuAHeaderSize = 2;
static const size_t kLengthFieldSize = 2;
static const size_t kStapAHeaderSize = kNalHeaderSize + kLengthFieldSize;
// Bit masks for FU (A and B) indicators.
enum NalDefs : uint8_t { kFBit = 0x80, kNriMask = 0x60, kTypeMask = 0x1F };

// Bit masks for FU (A and B) headers.
enum FuDefs : uint8_t { kSBit = 0x80, kEBit = 0x40, kRBit = 0x20 };
}
NonRtpPacketizerH264::Fragment::Fragment(const uint8_t *data_arg,int len_arg)
:buffer(data_arg)
,length(len_arg){}
NonRtpPacketizerH264::Fragment::Fragment(const Fragment&fragment){
	buffer=fragment.buffer;
	length=fragment.length;
}
NonRtpPacketizerH264::Fragment::~Fragment(){
}
NonRtpPacketizerH264::PacketUnit::PacketUnit(const Fragment &frag,bool first,bool last,
bool aggre,uint8_t head)
:fragment(frag)
,first_fragment(first)
,last_fragment(last)
,aggregate(aggre)
,header(head){}
NonRtpPacketizerH264::NonRtpPacketizerH264(ArrayView<const uint8_t>payload,PayloadSizeLimits limits,
		 H264PacketizationMode packetization_mode,const NonRtpFragmentHead&fragmentation){

	size_t i=0;
	for(i=0;i<fragmentation.fragmentVectorSize;i++){
		const uint8_t *fragment=payload.data()+fragmentation.fragmentOffset[i];
		int fragment_size=fragmentation.fragmentLength[i];
		input_fragments_.push_back(Fragment(fragment,fragment_size));
	}
	if(!GeneratePacket(packetization_mode)){
		while(!packets_.empty()){
			packets_.pop();
		}
	}
}
bool NonRtpPacketizerH264::NextPacket(NonRtpPacketToSend *rtp_packet){
	if(packets_.empty()){
		return false;
	}
	PacketUnit packet=packets_.front();
	if(packet.first_fragment&&packet.last_fragment){
		size_t byte_to_send=packet.fragment.length;
		uint8_t *buf=rtp_packet->AllocatePayload(byte_to_send);
		memcpy(buf,packet.fragment.buffer,byte_to_send);
		packets_.pop();
		input_fragments_.pop_front();
	}else if(packet.aggregate){
		NextAggregatePacket(rtp_packet);
	}else{
		NextFragmentPacket(rtp_packet);
	}
	num_packets_left_--;
	return true;
}
bool NonRtpPacketizerH264::GeneratePacket(H264PacketizationMode packetization_mode){
    for (size_t i = 0; i < input_fragments_.size();) {
    switch(packetization_mode){
    case H264PacketizationMode::SingleNalUnit:{
        if (!PacketizeSingleNalu(i))
          return false;
            ++i;
    	break;
    }
    case H264PacketizationMode::NonInterleaved:{
        int fragment_len = input_fragments_[i].length;
        int single_packet_capacity = limits_.max_payload_len;
        if (input_fragments_.size() == 1)
          single_packet_capacity -= limits_.single_packet_reduction_len;
        else if (i == 0)
          single_packet_capacity -= limits_.first_packet_reduction_len;
        else if (i + 1 == input_fragments_.size())
          single_packet_capacity -= limits_.last_packet_reduction_len;

        if (fragment_len > single_packet_capacity) {
          if (!PacketizeFuA(i))
            return false;
          	  ++i;
        } else {
          i = PacketizeStapA(i);
        }
    	break;
    }
    }
  }
  return true;
}
bool NonRtpPacketizerH264::PacketizeSingleNalu(size_t fragment_index){
	  // Add a single NALU to the queue, no aggregation.
	  size_t payload_size_left = limits_.max_payload_len;
	  if (input_fragments_.size() == 1)
	    payload_size_left -= limits_.single_packet_reduction_len;
	  else if (fragment_index == 0)
	    payload_size_left -= limits_.first_packet_reduction_len;
	  else if (fragment_index + 1 == input_fragments_.size())
	    payload_size_left -= limits_.last_packet_reduction_len;
	  const Fragment* fragment = &input_fragments_[fragment_index];
	  if (payload_size_left < fragment->length) {
	    DLOG(ERROR) << "Failed to fit a fragment to packet in SingleNalu "
	                         "packetization mode. Payload size left "
	                      << payload_size_left << ", fragment length "
	                      << fragment->length << ", packet capacity "
	                      << limits_.max_payload_len;
	    return false;
	  }
	  CHECK_GT(fragment->length, 0u);
	  packets_.push(PacketUnit(*fragment, true /* first */, true /* last */,
	                           false /* aggregated */, fragment->buffer[0]));
	  ++num_packets_left_;
	  return true;
}
bool NonRtpPacketizerH264::PacketizeFuA(size_t fragment_index){
	  // Fragment payload into packets (FU-A).
	  const Fragment& fragment = input_fragments_[fragment_index];

	  PayloadSizeLimits limits = limits_;
	  // Leave room for the FU-A header.
	  limits.max_payload_len -= kFuAHeaderSize;
	  // Update single/first/last packet reductions unless it is single/first/last
	  // fragment.
	  if (input_fragments_.size() != 1) {
	    // if this fragment is put into a single packet, it might still be the
	    // first or the last packet in the whole sequence of packets.
	    if (fragment_index == input_fragments_.size() - 1) {
	      limits.single_packet_reduction_len = limits_.last_packet_reduction_len;
	    } else if (fragment_index == 0) {
	      limits.single_packet_reduction_len = limits_.first_packet_reduction_len;
	    } else {
	      limits.single_packet_reduction_len = 0;
	    }
	  }
	  if (fragment_index != 0)
	    limits.first_packet_reduction_len = 0;
	  if (fragment_index != input_fragments_.size() - 1)
	    limits.last_packet_reduction_len = 0;

	  // Strip out the original header. and put it to FU-indicator
	  size_t payload_left = fragment.length - kNalHeaderSize;
	  int offset = kNalHeaderSize;
	  std::vector<int> payload_sizes=SplitAboutEqually(payload_left,limits);
	  if (payload_sizes.empty())
	    return false;

	  for (size_t i = 0; i < payload_sizes.size(); ++i) {
	    int packet_length = payload_sizes[i];
	    CHECK_GT(packet_length, 0);
	    packets_.push(PacketUnit(Fragment(fragment.buffer + offset, packet_length),
	                             /*first_fragment=*/i == 0,
	                             /*last_fragment=*/i == payload_sizes.size() - 1,
	                             false, fragment.buffer[0]));
	    offset += packet_length;
	    payload_left -= packet_length;
	  }
	  num_packets_left_ += payload_sizes.size();
	  CHECK_EQ(0, payload_left);
	  return true;
}
size_t NonRtpPacketizerH264::PacketizeStapA(size_t fragment_index){
	 // Aggregate fragments into one packet (STAP-A).
	  size_t payload_size_left = limits_.max_payload_len;
	  if (input_fragments_.size() == 1)
	    payload_size_left -= limits_.single_packet_reduction_len;
	  else if (fragment_index == 0)
	    payload_size_left -= limits_.first_packet_reduction_len;
	  int aggregated_fragments = 0;
	  size_t fragment_headers_length = 0;
	  const Fragment* fragment = &input_fragments_[fragment_index];
	  CHECK_GE(payload_size_left, fragment->length);
	  ++num_packets_left_;
	  auto payload_size_needed = [&] {
	    size_t fragment_size = fragment->length + fragment_headers_length;
	    if (input_fragments_.size() == 1) {
	      // Single fragment, single packet, payload_size_left already adjusted
	      // with limits_.single_packet_reduction_len.
	      return fragment_size;
	    }
	    if (fragment_index == input_fragments_.size() - 1) {
	      // Last fragment, so StrapA might be the last packet.
	      return fragment_size + limits_.last_packet_reduction_len;
	    }
	    return fragment_size;
	  };
	  if(aggregated_fragments==0){
		  fragment_headers_length+=(kNalHeaderSize + kLengthFieldSize);
	  }
	  while(payload_size_left>payload_size_needed()){
		  packets_.push(PacketUnit(*fragment,aggregated_fragments==0,
				  false,true,fragment->buffer[0]));
		  payload_size_left-=fragment->length;
		  payload_size_left-=fragment_headers_length;
		  CHECK_GE(fragment->length,0);
		    // If we are going to try to aggregate more fragments into this packet
		    // we need to add the STAP-A NALU header and a length field for the first
		    // NALU of this packet.
		  fragment_headers_length=kLengthFieldSize;
		  aggregated_fragments++;
		   // Next fragment.
		  ++fragment_index;
		  if (fragment_index == input_fragments_.size())
		     break;
		  fragment = &input_fragments_[fragment_index];
	  }
	  CHECK_GT(aggregated_fragments, 0);
	  packets_.back().last_fragment = true;
	  return fragment_index;
}
void NonRtpPacketizerH264::NextAggregatePacket(NonRtpPacketToSend *rtp_packet){
	  // Reserve maximum available payload, set actual payload size later.
	  size_t payload_capacity = rtp_packet->FreeCapacity();
	  CHECK_GE(payload_capacity, kNalHeaderSize);
	  uint8_t* buffer = rtp_packet->AllocatePayload(payload_capacity);
	  DCHECK(buffer);
	  PacketUnit* packet = &packets_.front();
	  CHECK(packet->first_fragment);
	  // STAP-A NALU header.
	  buffer[0] = (packet->header & (kFBit | kNriMask)) | H264::NaluType::kStapA;
	  size_t index = kNalHeaderSize;
	  bool is_last_fragment = packet->last_fragment;
	  while(packet->aggregate){
		  const Fragment &fragment=packet->fragment;
		  CHECK_LE(index + kLengthFieldSize + fragment.length, payload_capacity);
		  ByteWriter<uint16_t>::WriteBigEndian(&buffer[index], fragment.length);
		  index+=kLengthFieldSize;
		  memcpy(&buffer[index],fragment.buffer,fragment.length);
		  index+=fragment.length;
		  packets_.pop();
		  input_fragments_.pop_front();
		  if(is_last_fragment){
			  break;
		  }
		  packet = &packets_.front();
		  is_last_fragment=packet->last_fragment;
	  }
	  rtp_packet->SetPayloadSize(index);
}
void NonRtpPacketizerH264::NextFragmentPacket(NonRtpPacketToSend *rtp_packet){
	  PacketUnit* packet = &packets_.front();
	  // NAL unit fragmented over multiple packets (FU-A).
	  // We do not send original NALU header, so it will be replaced by the
	  // FU indicator header of the first packet.
	  uint8_t fu_indicator =
	      (packet->header & (kFBit | kNriMask)) | H264::NaluType::kFuA;
	  uint8_t fu_header = 0;

	  // S | E | R | 5 bit type.
	  fu_header |= (packet->first_fragment ? kSBit : 0);
	  fu_header |= (packet->last_fragment ? kEBit : 0);
	  uint8_t type = packet->header & kTypeMask;
	  fu_header |= type;
	  const Fragment& fragment = packet->fragment;
	  uint8_t* buffer =
	      rtp_packet->AllocatePayload(kFuAHeaderSize + fragment.length);
	  buffer[0] = fu_indicator;
	  buffer[1] = fu_header;
	  memcpy(buffer + kFuAHeaderSize, fragment.buffer, fragment.length);
	  if (packet->last_fragment)
	    input_fragments_.pop_front();
	  packets_.pop();
}
}

#include "logging.h"
#include "rtc_recv_stream.h"
namespace zsy{
const char start3[]={0x00,0x00,0x01};
const char start4[]={0x00,0x00,0x00,0x01};
const uint8_t kTypeMask = 0x1F;
static const size_t kLengthFieldSize = 2;
enum FuDefs : uint8_t { kSBit = 0x80, kEBit = 0x40, kRBit = 0x20 };
void ParseNaluTypeFromPacket(const uint8_t *data,size_t size,std::vector<uint8_t> &types){
	if(size<1){
		return;
	}
	uint8_t nalu_type=0;
	uint8_t packet_type=data[0]&kTypeMask;
	if(packet_type==H264::NaluType::kStapA){
		size_t offset=1;
		while(offset<size){
			uint16_t length=ByteReader<uint16_t>::ReadBigEndian(data+offset);
			offset+=kLengthFieldSize;
			nalu_type=data[offset]&kTypeMask;
			offset+=length;
			types.push_back(nalu_type);
		}
	}else if((packet_type==H264::NaluType::kFuA)&&size>=2){
		//FU header
		nalu_type=data[1]&kTypeMask;
		types.push_back(nalu_type);
	}else{
		nalu_type=data[0]&kTypeMask;
		types.push_back(nalu_type);
	}
	return ;
}
RecvNalu& RecvNalu::operator=(RecvNalu&&r){
	packetizer_mode_=r.packetizer_mode_;
	nalu_type_=r.nalu_type_;
	start_code_len_=r.start_code_len_;
	timestamp_=r.timestamp_;
	total_length_=r.total_length_;
	r.total_length_=0;
	payloads_=std::move(r.payloads_);
	lengths_=std::move(r.lengths_);
	return *this;
}
void RecvNalu::OnNewPacket(const uint8_t *data,int length){
	payloads_.push_back(data);
	lengths_.push_back(length);
	total_length_+=length;
}
void RecvNalu::SetPacketizerMode(uint8_t mode){
	if(packetizer_mode_==0){
		packetizer_mode_=mode;
	}
}
RecvFrame::~RecvFrame(){
	while(!packets_.empty()){
		NonRtpPacket *packet=nullptr;
		auto it=packets_.begin();
		packet=it->second;
		delete packet;
		packets_.erase(it);
	}
}
void RecvFrame::OnNewPacket(uint64_t offset,uint32_t len,NonRtpPacket *packet){
	if(packets_of_frame_==0){
		frame_type_=packet->PayloadType();
		packets_of_frame_=packet->PacketsPerFrame();
		capture_ts_=packet->Timestamp();
	}
	ArrayView<const uint8_t> payload=packet->payload();
	packets_.emplace(OffsetAndLength(offset,len),packet);
	received_++;
}
bool RecvFrame::IsFullFrame() const{
	bool full=false;
	if(packets_of_frame_!=0){
		if(packets_of_frame_==received_){
			full=true;
		}
	}
	return full;
}
uint64_t RecvFrame::GetOffsetAfterConsume() const{
	uint64_t offset=0;
	if(!packets_.empty()){
		auto it=packets_.rbegin();
		offset=it->first.offset+it->first.length;
	}
	return offset;
}
uint32_t RecvFrame::GetDelivableSize(){
	uint32_t deliver_size=0;
	bool parse_succeed=true;
	for(auto packet_it=packets_.begin();packet_it!=packets_.end();packet_it++){
		bool ret=ParserPacketToRecvNalu(packet_it->second,nalus_);
		if(!ret){
			parse_succeed=false;
			break;
		}
	}
	if(parse_succeed){
		for(auto it=nalus_.begin();it!=nalus_.end();it++){
			int addon=0;
			if(it->GetPacketizerMode()==H264::NaluType::kFuA){
				addon=1;
			}
			deliver_size+=(it->start_code_len_+it->total_length_+addon);
		}
	}
	return deliver_size;
}
bool RecvFrame::ParserStapAToRecvNalu(NonRtpPacket *packet,std::vector<RecvNalu>&nalus){
	ArrayView<const uint8_t> payload=packet->payload();
	size_t size=payload.size();
	const uint8_t *data=payload.data();
	uint8_t packet_type=data[0]&kTypeMask;
	if(packet_type!=H264::NaluType::kStapA){
		DLOG(INFO)<<"parse stapa error";
		return false;
	}
	uint32_t timestamp=packet->Timestamp();
	size_t offset=1;
	uint8_t start_code_len=3;
	while(offset<size){
		start_code_len=3;
		RecvNalu nalu;
		uint16_t length=ByteReader<uint16_t>::ReadBigEndian(data+offset);
		offset+=kLengthFieldSize;
		uint8_t nalu_type=data[offset]&kTypeMask;
		nalu.nalu_type_=nalu_type;
		nalu.OnNewPacket(data+offset,length);
		nalu.timestamp_=timestamp;
		if(nalu_type==H264::kSps||nalu_type==H264::kPps){
			start_code_len=4;
		}
		nalu.start_code_len_=start_code_len;
		nalu.SetPacketizerMode(H264::NaluType::kStapA);
		nalus.push_back(std::move(nalu));
		offset+=length;
	}
	return true;
}
bool RecvFrame::ParserFuAToRecvNalu(NonRtpPacket *packet,std::vector<RecvNalu>&nalus){
	ArrayView<const uint8_t> payload=packet->payload();
	size_t size=payload.size();
	const uint8_t *data=payload.data();
	uint8_t fu_indicator=data[0];
	uint8_t packet_type=fu_indicator&kTypeMask;
	if(packet_type!=H264::NaluType::kFuA){
		DLOG(INFO)<<"parse fua error";
		return false;
	}
	uint8_t fu_header=data[1];
	uint8_t nalu_type=fu_header&kTypeMask;
	uint32_t timestamp=packet->Timestamp();
	size_t header_length=2;
	size_t nalu_size=size-header_length;
	uint8_t start_code_len=3;
	bool first_packet=fu_header&kSBit;
	if(first_packet){
        start_code_len=4;
        //for sps pps idr
        if(!nalus.empty()){
        	auto it=nalus.rbegin();
        	if(timestamp==it->timestamp_){
        		if(it->start_code_len_==4||it->start_code_len_==3){
        			start_code_len=3;
        		}
        		if(it->nalu_type_==H264::kSps||it->nalu_type_==H264::kPps||
        				it->nalu_type_==H264::kSei){
        			start_code_len=3;
        		}

        	}
        }
        //in case the sei nalu is too large and packetized as FUA.
        if(nalu_type==H264::kSei){
        	start_code_len=3;
        }
        RecvNalu nalu;
        nalu.nalu_type_=nalu_type;
        nalu.OnNewPacket(data+header_length,nalu_size);
        nalu.timestamp_=timestamp;
        nalu.start_code_len_=start_code_len;
        nalu.SetPacketizerMode(H264::NaluType::kFuA);
        nalus.push_back(std::move(nalu));
	}else{
		CHECK(!nalus.empty());
		auto it=nalus.rbegin();
		if(timestamp==it->timestamp_){
			it->OnNewPacket(data+header_length,nalu_size);
		}else{
			DLOG(INFO)<<"parse fua packet lost";
			return false;
		}
	}
	return true;
}
bool RecvFrame::ParserPacketToRecvNalu(NonRtpPacket *packet,std::vector<RecvNalu>&nalus){
	ArrayView<const uint8_t> payload=packet->payload();
	size_t payload_size=payload.size();
	uint8_t type=payload[0]&kTypeMask;
	if(type==H264::NaluType::kStapA){
		return ParserStapAToRecvNalu(packet,nalus);
	}else if(type==H264::NaluType::kFuA){
		return ParserFuAToRecvNalu(packet,nalus);
	}
	return false;
}
RTCRecvStream::RTCRecvStream(std::string &s){
	f_out_.open(s.c_str(),std::fstream::out);
}
RTCRecvStream::~RTCRecvStream(){
	if(f_out_.is_open()){
		f_out_.close();
	}
}
void RTCRecvStream::OnNewData(uint64_t offset,const uint8_t *data,size_t size){
	NonRtpPacket *parser=new NonRtpPacket();
	bool ret=false;
	ret=parser->Parse(data,size);
	if(ret){
		uint32_t payload_type=parser->PayloadType();
		uint32_t packets_per_frame=parser->PacketsPerFrame();
		uint32_t group_id=group_id_.Unwrap(parser->GroupId());
		ArrayView<const uint8_t> payload=parser->payload();
		uint32_t timestamp=parser->Timestamp();
		size_t payload_size=payload.size();
		uint32_t type=payload[0]&kTypeMask;
		if(f_out_.is_open()){
			std::vector<uint8_t> nalu_types;
			ParseNaluTypeFromPacket(payload.data(),payload_size,nalu_types);
			f_out_<<payload_type<<" "<<type<<" "<<packets_per_frame<<" "<<
					group_id<<" "<<payload_size<<" ";
			for(auto it=nalu_types.begin();it!=nalu_types.end();it++){
				uint32_t nalu_type=(*it);
				f_out_<<nalu_type<<" ";
			}
			f_out_<<std::endl;
		}
		if(offset>=last_comsumed_offset_){
			if(frames_.empty()){
				RecvFrame *frame=new RecvFrame();
				frame->OnNewPacket(offset,size,parser);
				frames_.insert(std::make_pair(timestamp,frame));
			}else{
				auto it=frames_.find(timestamp);
				if(it!=frames_.end()){
					RecvFrame *frame=it->second;
					frame->OnNewPacket(offset,size,parser);
				}else{
					RecvFrame *frame=new RecvFrame();
					frame->OnNewPacket(offset,size,parser);
					frames_.insert(std::make_pair(timestamp,frame));
				}
			}
		}else{
			delete parser;
		}
	}else{
		delete parser;
		DLOG(INFO)<<"packet parser error";
	}
	CheckDeliverable();
}
void RTCRecvStream::CheckDeliverable(){
	while(!frames_.empty()){
		auto it=frames_.begin();
		RecvFrame *frame=it->second;
		uint32_t timestamp=it->first;
		if(frame->IsFullFrame()){
			uint32_t read_size=frame->GetDelivableSize();
			std::cout<<"read size "<<read_size<<std::endl;
			uint32_t nalus=frame->GetNaluSize();
			for(size_t i=0;i<nalus;i++){
				uint32_t start_code_len=frame->GetStatCodeLenOfNalu(i);
				std::cout<<"nalu "<<(i+1)<<" stl "<<start_code_len<<std::endl;
			}

			frames_.erase(it);
			delete frame;
		}else{
			uint32_t recv=frame->Received();
			uint32_t packets_per_frame=frame->PacketsPerFrame();
			break;
		}
	}
}
}

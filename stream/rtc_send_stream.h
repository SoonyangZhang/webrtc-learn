#include "base/my_thread.h"
#include "base/atomic_ops.h"
#include "base/lock.h"
#include "endecoder/videocodec.h"
#include "packetizer/non_rtp_packet_to_send.h"
#include <stdint.h>
#include <list>
#include <map>
namespace zsy{
class RTCRecvStream;
class RTCSendStream:public EncodedVideoCallback,public MyThread{
public:
	struct SendPacketInfo{
	SendPacketInfo(uint64_t o,int16_t l,int t,uint8_t ty,uint8_t r)
	:offset(o),sent_time(t),length(l),type(ty),retrans(r){}
	void Reset(uint64_t o,int16_t l,int t,uint8_t ty,uint8_t r){
	    offset=(o);sent_time=(t);length=(l);type=(ty);retrans=(r);
	}
	void SetData(NonRtpPacketToSend *data){buffer=data;}
	void DeleteBuffer(){
	    if(buffer){
	        delete buffer;
	        buffer=nullptr;
	    }
	}
	uint64_t offset;
	uint32_t sent_time;
	uint16_t length;
	uint8_t type:4;
	uint8_t retrans:4;
	NonRtpPacketToSend *buffer{nullptr};
	};
	RTCSendStream();
	~RTCSendStream();
	void StartSender();
	void StopSender();
	void Run() override;
	void OnEncodedImageCallBack(EncodeImage &image) override;
	bool HasDataToSend() const;
	void RegisterReceiver(RTCRecvStream *sink){
		target_=sink;
	}
private:
	void PakcetizeImage();
	void SendPacket();
	bool running_{true};
	bool first_frame_{true};
	uint16_t group_id_{0};
	uint16_t frame_id_{0};
	int64_t offset_{0};
	int64_t wait_send_offset_{0};
	AtomicLock que_lock_;
	uint32_t image_count_{0};
	std::list<EncodeImage> images_;
	std::map<int64_t,SendPacketInfo> packets_;
	RTCRecvStream *target_{nullptr};
};
}

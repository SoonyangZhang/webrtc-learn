#pragma once
#include "non_rtp_packet.h"
namespace zsy{
class NonRtpPacketToSend:public NonRtpPacket{
public:
 enum class Type {
   kAudio,                   // Audio media packets.
   kVideo,                   // Video media packets.
   kRetransmission,          // RTX (usually) packets send as response to NACK.
   kForwardErrorCorrection,  // FEC packets.
   kPadding                  // RTX or plain padding sent to maintain BWE.
 };
 NonRtpPacketToSend &operator=(const NonRtpPacketToSend&);
 NonRtpPacketToSend &operator=(NonRtpPacketToSend&&);
 // Time in local time base as close as it can to frame capture time.
 int64_t capture_time_ms() const { return capture_time_ms_; }

 void set_capture_time_ms(int64_t time) { capture_time_ms_ = time; }

 void set_packet_type(Type type) { packet_type_ = type; }
 Type packet_type() const { return packet_type_; }
private:
 int64_t capture_time_ms_{0};
 Type packet_type_;
};
}

#pragma once
#include <stdint.h>
#include <vector>
namespace zsy{
// Packetization modes are defined in RFC 6184 section 6
// Due to the structure containing this being initialized with zeroes
// in some places, and mode 1 being default, mode 1 needs to have the value
// zero. https://crbug.com/webrtc/6803
enum class H264PacketizationMode {
  NonInterleaved = 0,  // Mode 1 - STAP-A, FU-A is allowed
  SingleNalUnit        // Mode 0 - only single NALU allowed
};
enum PayloadType:uint8_t{
	PT_H264=96,
};
struct PayloadSizeLimits {
  int max_payload_len = 1200;
  int first_packet_reduction_len = 0;
  int last_packet_reduction_len = 0;
  // Reduction len for packet that is first & last at the same time.
  int single_packet_reduction_len = 0;
};
class NonRtpFragmentHead{
public:
	NonRtpFragmentHead(uint16_t fragments);
	~NonRtpFragmentHead();
	NonRtpFragmentHead(const NonRtpFragmentHead&)=delete;
	NonRtpFragmentHead &operator=(const NonRtpFragmentHead&)=delete;
	uint16_t fragmentVectorSize{0};
	int *fragmentOffset;
	int *fragmentLength;
};
std::vector<int> SplitAboutEqually(int payload_len,
		const PayloadSizeLimits &limits);
}

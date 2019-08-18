#include <deque>
#include <queue>
#include "array_view.h"
#include "non_rtp.h"
#include "non_rtp_packet_to_send.h"
namespace zsy{
class NonRtpPacketizerH264{
public:
	NonRtpPacketizerH264(ArrayView<const uint8_t>payload,PayloadSizeLimits limits,
			 H264PacketizationMode packetization_mode,const NonRtpFragmentHead&fragmentation);
	bool NextPacket(NonRtpPacketToSend *rtp_packet);
    int num_packet_left() const {return num_packets_left_;}
private:
	struct Fragment{
	Fragment(const uint8_t *data_arg,int len_arg);
	explicit Fragment(const Fragment&fragment);
	~Fragment();
	const uint8_t *buffer{nullptr};
	int length;
	};
	struct PacketUnit{
	PacketUnit(const Fragment &frag,bool first,bool last,
			bool aggre,uint8_t head);
	const Fragment fragment;
	bool first_fragment;
	bool last_fragment;
	bool aggregate;
	uint8_t header;
	};
	bool PacketizeSingleNalu(size_t fragment_index);
	bool GeneratePacket(H264PacketizationMode packetization_mode);
	bool PacketizeFuA(size_t fragment_index);
	size_t PacketizeStapA(size_t fragment_index);
	void NextAggregatePacket(NonRtpPacketToSend *rtp_packet);
	void NextFragmentPacket(NonRtpPacketToSend *rtp_packet);
	PayloadSizeLimits limits_;
	std::deque<Fragment> input_fragments_;
	std::queue<PacketUnit> packets_;
	int num_packets_left_{0};
};
}

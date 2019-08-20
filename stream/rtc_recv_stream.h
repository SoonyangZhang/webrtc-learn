#pragma once
#include <stdint.h>
#include <string>
#include <fstream>
namespace zsy{
class RTCRecvStream{
public:
	RTCRecvStream(std::string &s);
	~RTCRecvStream();
	void OnNewData(const uint8_t *data,size_t size);
private:
	int64_t frame_id_{-1};
	int64_t group_id_{-1};
	std::fstream f_out_;
};
}

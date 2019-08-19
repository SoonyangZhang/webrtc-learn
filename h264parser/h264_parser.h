#pragma once
#include <stdio.h>
#include <stdint.h>
namespace zsy{
struct H264NALU{
const uint8_t *data;
size_t size;
size_t start_code_size;
};
class H264Parser{
public:
  enum Result {
    kOk,
    kInvalidStream,      // error in stream
    kUnsupportedStream,  // stream not supported by the parser
    kEOStream,           // end of stream
  };
void SetStream(const uint8_t* stream, size_t stream_size);
H264Parser::Result AdvanceToNextNALU(H264NALU* nalu);
bool LocateNALU(size_t* nalu_size, size_t* start_code_size);
static bool FindStartCode(const uint8_t* data,
                            size_t data_size,
                            size_t* offset,
                            size_t* start_code_size);
private:
    const uint8_t *stream_{nullptr};
    size_t bytes_left_;
};
}

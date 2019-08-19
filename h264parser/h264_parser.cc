#include <memory.h>
#include "h264_parser.h"
#include "logging.h"
#define DVLOG(x) VLOG(x)
namespace zsy{
static inline bool IsStartCode(const uint8_t* data) {
  return data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x01;
}
void H264Parser::SetStream(const uint8_t* stream, size_t stream_size){
    stream_=stream;
    bytes_left_=stream_size;
}
H264Parser::Result H264Parser::AdvanceToNextNALU(H264NALU* nalu){
  size_t start_code_size;
  size_t nalu_size_with_start_code;
  if (!LocateNALU(&nalu_size_with_start_code, &start_code_size)) {
    DVLOG(4) << "Could not find next NALU, bytes left in stream: "
             << bytes_left_;
    stream_ = nullptr;
    bytes_left_ = 0;
    return kEOStream;
  }

  nalu->data = stream_ + start_code_size;
  nalu->size = nalu_size_with_start_code - start_code_size;
  nalu->start_code_size=start_code_size;
  DVLOG(4) << "NALU found: size=" << nalu_size_with_start_code;
  // Move parser state to after this NALU, so next time AdvanceToNextNALU
  // is called, we will effectively be skipping it;
  // other parsing functions will use the position saved
  // in bit reader for parsing, so we don't have to remember it here.
  stream_ += nalu_size_with_start_code;
  bytes_left_ -= nalu_size_with_start_code;
  return kOk;
}
bool H264Parser::LocateNALU(size_t* nalu_size, size_t* start_code_size){
  // Find the start code of next NALU.
  size_t nalu_start_off = 0;
  size_t annexb_start_code_size = 0;

  if (!FindStartCode(stream_, bytes_left_,
                                  &nalu_start_off, &annexb_start_code_size)) {
    DVLOG(4) << "Could not find start code, end of stream?";
    return false;
  }

  // Move the stream to the beginning of the NALU (pointing at the start code).
  stream_ += nalu_start_off;
  bytes_left_ -= nalu_start_off;

  const uint8_t* nalu_data = stream_ + annexb_start_code_size;
  off_t max_nalu_data_size = bytes_left_ - annexb_start_code_size;
  if (max_nalu_data_size <= 0) {
    DVLOG(3) << "End of stream";
    return false;
  }

  // Find the start code of next NALU;
  // if successful, |nalu_size_without_start_code| is the number of bytes from
  // after previous start code to before this one;
  // if next start code is not found, it is still a valid NALU since there
  // are some bytes left after the first start code: all the remaining bytes
  // belong to the current NALU.
  size_t next_start_code_size = 0;
  size_t nalu_size_without_start_code = 0;
  if (!FindStartCode(
          nalu_data, max_nalu_data_size,
          &nalu_size_without_start_code, &next_start_code_size)) {
    nalu_size_without_start_code = max_nalu_data_size;
  }
  *nalu_size = nalu_size_without_start_code + annexb_start_code_size;
  *start_code_size = annexb_start_code_size;
  return true;
}
bool H264Parser::FindStartCode(const uint8_t* data,
                            size_t data_size,
                            size_t* offset,
                            size_t* start_code_size){
  size_t bytes_left = data_size;

  while (bytes_left >= 3) {
    // The start code is "\0\0\1", ones are more unusual than zeroes, so let's
    // search for it first.
    const uint8_t* tmp =
        reinterpret_cast<const uint8_t*>(memchr(data + 2, 1, bytes_left - 2));
    if (!tmp) {
      data += bytes_left - 2;
      bytes_left = 2;
      break;
    }
    tmp -= 2;
    bytes_left -= tmp - data;
    data = tmp;

    if (IsStartCode(data)) {
      // Found three-byte start code, set pointer at its beginning.
      *offset = data_size - bytes_left;
      *start_code_size = 3;

      // If there is a zero byte before this start code,
      // then it's actually a four-byte start code, so backtrack one byte.
      if (*offset > 0 && *(data - 1) == 0x00) {
        --(*offset);
        ++(*start_code_size);
      }

      return true;
    }

    ++data;
    --bytes_left;
  }

  // End of data: offset is pointing to the first byte that was not considered
  // as a possible start of a start code.
  // Note: there is no security issue when receiving a negative |data_size|
  // since in this case, |bytes_left| is equal to |data_size| and thus
  // |*offset| is equal to 0 (valid offset).
  *offset = data_size - bytes_left;
  *start_code_size = 0;
  return false;
}
}

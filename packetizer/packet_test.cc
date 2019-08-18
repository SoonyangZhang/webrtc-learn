#include "array_view.h"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include "non_rtp_h264.h"
#include "non_rtp.h"
#include "copy_on_write_buffer.h"
#include "byte_rw.h"
#include "non_rtp_packet_to_send.h"
using namespace zsy;
using namespace std;
struct Test{
Test(uint32_t header):header(header){
    //header=header;
}

uint32_t header{0};
};
int main(){
    uint8_t data[2000]={0};
    NonRtpFragmentHead header(3);
    header.fragmentLength[0]=200;
    header.fragmentOffset[0]=0;
    header.fragmentLength[1]=300;
    header.fragmentOffset[1]=200;
    header.fragmentLength[2]=1500;
    header.fragmentOffset[2]=500;
    ArrayView<const uint8_t> payload(data,2000);
    PayloadSizeLimits limits;

    NonRtpPacketizerH264  h264_packetizer(payload,limits,
                                          H264PacketizationMode::NonInterleaved,header);

    NonRtpPacketToSend rtp_send;

    rtp_send.ReserveHeaderSpace();
    printf("left %d\n",h264_packetizer.num_packet_left());
    bool ret=false;
    ret=h264_packetizer.NextPacket(&rtp_send);
    if(ret){
    uint8_t type=*(rtp_send.ReadAt(5));
    uint32_t value=type;
    uint16_t tmp=ByteReader<uint16_t>::ReadBigEndian(rtp_send.ReadAt(6));
    uint32_t len=tmp;
    size_t size=rtp_send.size();

    printf("%x len %d %d\n",value,len,size);
    }
    printf("left %d\n",h264_packetizer.num_packet_left());
    return 0;
}

#include "array_view.h"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <list>
#include "packetizer/non_rtp_h264.h"
#include "packetizer/non_rtp.h"
#include "base/copy_on_write_buffer.h"
#include "base/byte_rw.h"
#include "packetizer/non_rtp_packet_to_send.h"
#include "logging.h"
using namespace zsy;
using namespace std;

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

    std::unique_ptr<NonRtpPacketToSend> rtp_send(new NonRtpPacketToSend());

    rtp_send->ReserveHeaderSpace();
    rtp_send->SetPayloadType(1);
    rtp_send->SetTimestamp(1234);
    rtp_send->SetGroupId(23);
    rtp_send->SetFrameId(78);
    printf("left %d\n",h264_packetizer.num_packet_left());
    bool ret=false;
    ret=h264_packetizer.NextPacket(rtp_send.get());
    if(ret){
    uint8_t type=*(rtp_send->ReadAt(9));
    uint32_t value=type;
    uint16_t tmp=ByteReader<uint16_t>::ReadBigEndian(rtp_send->ReadAt(10));
    uint32_t len=tmp;
    size_t size=rtp_send->size();
    printf("%x len %d %d\n",value,len,size);
    }
    printf("left %d\n",h264_packetizer.num_packet_left());
    if(rtp_send){
    	DLOG(INFO)<<"ref "<<rtp_send->GetRefCount();
    }
    const uint8_t *ptr=rtp_send->data();
    NonRtpPacket parser;
    ret=parser.Parse(ptr,rtp_send->size());
    if(ret){
    	std::cout<<parser.Timestamp()<<" "<<parser.GroupId()<<" "
    			<<parser.FrameId()<<std::endl;
    }
    return 0;
}

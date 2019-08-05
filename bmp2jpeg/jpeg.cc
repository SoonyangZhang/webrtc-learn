#include <stdio.h>
#include <stdlib.h>
#include "jpeg.h"
#include "dct.h"
#include "huffencode.h"

/*将SOI写入JPEG文件*/
void writeSOI(OutputStream *ops)
{
	WriteDirect(ops, SOI, sizeof(SOI));
}

/*将APP0写入JPEG文件*/
void writeAPP0(OutputStream *ops)
{
	WriteDirect(ops, APP0, sizeof(APP0));
}

/*将JPEG量化表申明写入JPEG文件*/
void writeDQT(OutputStream *ops, DWORD param1)
{
	DQT[4] = (BYTE)param1;
	WriteDirect(ops, DQT, sizeof(DQT));
}

/*将亮度和色度量化表写入JPEG文件*/
void writeQuantTable(OutputStream *ops)
{
	int i,j;
	BYTE zigQuant[DCT_SIZE*DCT_SIZE];

	writeDQT(ops,0);
	for (i=0; i<DCT_SIZE; i++)
		for (j=0; j<DCT_SIZE; j++)
   			zigQuant[zigZagTable[i][j]] = lumQuantTable[i][j];
	WriteDirect(ops, zigQuant, DCT_SIZE*DCT_SIZE);

	writeDQT(ops,1);
	for (i=0; i<DCT_SIZE; i++)
		for (j=0; j<DCT_SIZE; j++)
   			zigQuant[zigZagTable[i][j]] = chrQuantTable[i][j];
	WriteDirect(ops, zigQuant, DCT_SIZE*DCT_SIZE);
}

/*将SOF0写入JPEG文件*/
void writeSOF0(OutputStream *ops, DWORD param1, DWORD param2)
{
	SOF0[5] = (BYTE)(param1/256);
  	SOF0[6] = (BYTE)(param1%256);
    SOF0[7] = (BYTE)(param2/256);
  	SOF0[8] = (BYTE)(param2%256);
    WriteDirect(ops, SOF0, sizeof(SOF0));
}

/*将DHC写入JPEG文件*/
void writeDHT(OutputStream *ops, DWORD param1, DWORD param2)
{
	DHT[2] = (BYTE)((3+param1)/256);
	DHT[3] = (BYTE)((3+param1)%256);
	DHT[4] = (BYTE)param2;
    WriteDirect(ops, DHT, sizeof(DHT));
}

/*将Huffman表写入JPEG文件*/
void writeHuffmanTable(OutputStream *ops, MainInfo *mi)
{
	writeSOF0(ops,mi->height, mi->width);

	writeDHT(ops, sizeof(huffmanLumDCBit)-1+sizeof(huffmanLumDCVal), 0x00);
	WriteDirect(ops, huffmanLumDCBit+1, sizeof(huffmanLumDCBit)-1);
	WriteDirect(ops, huffmanLumDCVal, sizeof(huffmanLumDCVal));

	writeDHT(ops, sizeof(huffmanLumACBit)-1+sizeof(huffmanLumACVal), 0x10);
	WriteDirect(ops, huffmanLumACBit+1, sizeof(huffmanLumACBit)-1);
	WriteDirect(ops, huffmanLumACVal, sizeof(huffmanLumACVal));

	writeDHT(ops, sizeof(huffmanChrDCBit)-1+sizeof(huffmanChrDCVal), 0x01);
	WriteDirect(ops, huffmanChrDCBit+1, sizeof(huffmanChrDCBit)-1);
	WriteDirect(ops, huffmanChrDCVal, sizeof(huffmanChrDCVal));

	writeDHT(ops, sizeof(huffmanChrACBit)-1+sizeof(huffmanChrACVal), 0x11);
	WriteDirect(ops, huffmanChrACBit+1, sizeof(huffmanChrACBit)-1);
	WriteDirect(ops, huffmanChrACVal, sizeof(huffmanChrACVal));

	WriteDirect(ops, SOS, sizeof(SOS));
}

/*将EOI写入JPEG文件*/
void writeEOI(OutputStream *ops)
{
	WriteDirect(ops, EOI, sizeof(EOI));
}

#include <stdio.h>
#include <stdlib.h>
#include "jpeg.h"
#include "dct.h"
#include "huffencode.h"

/*��SOIд��JPEG�ļ�*/
void writeSOI(OutputStream *ops)
{
	WriteDirect(ops, SOI, sizeof(SOI));
}

/*��APP0д��JPEG�ļ�*/
void writeAPP0(OutputStream *ops)
{
	WriteDirect(ops, APP0, sizeof(APP0));
}

/*��JPEG����������д��JPEG�ļ�*/
void writeDQT(OutputStream *ops, DWORD param1)
{
	DQT[4] = (BYTE)param1;
	WriteDirect(ops, DQT, sizeof(DQT));
}

/*�����Ⱥ�ɫ��������д��JPEG�ļ�*/
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

/*��SOF0д��JPEG�ļ�*/
void writeSOF0(OutputStream *ops, DWORD param1, DWORD param2)
{
	SOF0[5] = (BYTE)(param1/256);
  	SOF0[6] = (BYTE)(param1%256);
    SOF0[7] = (BYTE)(param2/256);
  	SOF0[8] = (BYTE)(param2%256);
    WriteDirect(ops, SOF0, sizeof(SOF0));
}

/*��DHCд��JPEG�ļ�*/
void writeDHT(OutputStream *ops, DWORD param1, DWORD param2)
{
	DHT[2] = (BYTE)((3+param1)/256);
	DHT[3] = (BYTE)((3+param1)%256);
	DHT[4] = (BYTE)param2;
    WriteDirect(ops, DHT, sizeof(DHT));
}

/*��Huffman��д��JPEG�ļ�*/
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

/*��EOIд��JPEG�ļ�*/
void writeEOI(OutputStream *ops)
{
	WriteDirect(ops, EOI, sizeof(EOI));
}
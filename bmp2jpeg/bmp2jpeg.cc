#include <stdio.h>
#include <string.h>
#include "datatype.h"
#include "encode.h"


int main()
{
	char bmp[16], jpeg[16];
	bool isBMP;

	strcpy(bmp,"test.bmp");
	strcpy(jpeg,"test.jpeg");

	//verify whether input file is .bmp
	isBMP = EncodeWholeFile(jpeg, bmp);

	if( !isBMP )
	{
		printf("file open error\n");
	}
    else
    {
       	printf("convert successfully\n");
    }

	return 0;
}
/*
int main(){
    printf("%d,%d,%d\n",sizeof(WORD),sizeof(DWORD),sizeof(LONG));
    printf("%d,%d\n",sizeof(BITMAPFILEHEADER),sizeof(BITMAPINFOHEADER));
    return 0;
}*/

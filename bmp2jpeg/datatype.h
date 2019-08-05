#ifndef	DATATYPE_H
#define DATATYPE_H
#include <stdint.h>
typedef	unsigned char	BYTE;
typedef unsigned short WORD;
#if defined linux_plat
typedef unsigned int DWORD;
typedef int LONG;
#else
#include <windows.h>
#endif


#endif

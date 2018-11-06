#ifndef __COMMON_INCLUDE_H__
#define __COMMON_INCLUDE_H__

#include <stdio.h>
#include <stdlib.h>

#define COMM_MALLOC(p, s) \
	if(!((p) = malloc(s))) { \
		fprintf(stderr, "Insufficient memory!"); \
		exit(EXIT_FAILURE); \
	}

/*
	void *calloc(size_t n, size_t size);
	在内存的动态存储区中分配n个长度为size的连续空间，函数返回一个指向分配起始地址的指针；如果分配不成功，返回NULL。
	与malloc的区别：
		alloc在动态分配完内存后，自动初始化该内存空间为零，而malloc不初始化，里边数据是随机的垃圾数据。
*/
#define COMM_CALLOC(p, n, s) \
	if (!((p) = calloc((n), (s)))) { \
		printf(stderr, "Insufficient memory!"); \
		exit(EXIT_FAILURE); \
	}

/*
	void *realloc(void *mem_address, unsigned int newsize);
	指针名 =（数据类型*）realloc（要改变内存大小的指针名，新的大小）。
	新的大小可大可小（但是要注意，如果新的大小小于原内存大小，可能会导致数据丢失，慎用！）

	先判断当前的指针是否有足够的连续空间，如果有，扩大mem_address指向的地址，并且将mem_address返回，如果空间不够，
	先按照newsize指定的大小分配空间，将原有数据从头到尾拷贝到新分配的内存区域，而后释放原来mem_address所指内存区域
	（注意：原来指针是自动释放，不需要使用free），同时返回新分配的内存区域的首地址。即重新分配存储器块的地址。
	返回值
	如果重新分配成功则返回指向被分配内存的指针，否则返回空指针NULL。
*/
#define COMM_REALLOC(p, s) \
	if (!((p) = realloc((p), (s)))) { \
		printf(stderr, "Insufficient memory!"); \
		exit(EXIT_FAILURE); \
	}

// 交换-所有变量类型
#define COMM_SWAP(x, y, t) ((t) = (x), (x) = (y), (y) = (t))

// 比较两个
#define  COMM_COMPARE(x, y) (((x) < (y)) ? -1 : ((x) == (y)) ? 0 : 1)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
// [2进制]大小端判定
int g_endian = 0x01020304;

目前Intel的80x86系列芯片 小端存储, 网络上传输数据普遍采用的都是大端.

// 内存 低位->高位
// 小端方式(地址增长顺序和值增长顺序相同，高字节高位存储): 04 03 02 01
// 大端方式(地址增长顺序和值增长顺序相反，高字节低位存储): 01 02 03 04

union
{
	int a;
	char b;
} endian;

// 1、指针方式判定
char *p = (char*)&g_endian;
// *p == 4 小端 *p== 1 大端
printf("ret = [%d]\n", *p);
printf("&a = [%p]\n", &g_endian);

// 2、联合体方式判定
endian.a = g_endian;
printf("ret = [%d]\n", endian.b);
*/

// 大小端转换 [如32位整数]
// 1、宏定义
#define ENDIAN_TRANSFER(x) ( (((x)&0xff)<<24) | (((x)&0xff00)<<8) | (((x)&0xff0000)>>8) | (((x)&0xff000000)>>24) )
// 2、函数式 [字符类型转换的优先级高于移位]
inline void EndianTransfer(int x)
{
	printf("before transfered x is 0x%x\n", x);

	char a, b, c, d;
	a = (char)(x & 0xff);
	b = (char)((x & 0xff00) >> 8);
	c = (char)((x & 0xff0000) >> 16);
	d = (char)((x & 0xff000000) >> 24);

	printf("0x%x 0x%x 0x%x 0x%x\n", a, b, c, d);

	x = (a << 24) | (b << 16) | (c << 18) | d;

	printf("after transfered x is 0x%x\n", x);
}
////////////////////////cocos2dx中的方法//////////////////////////////////////////////////
/**
Helper macros which converts 4-byte little/big endian
integral number to the machine native number representation

It should work same as apples CFSwapInt32LittleToHost(..)
*/

/// when define returns true it means that our architecture uses big endian
// 小端： 地址低->高 00 ff
// 实际转成 unsigned short: 0xff00 = 65280
// > 0x100 内存存放：地址低->高 00 01
///
// 大端：地址低->高 ff 00
// 实际 0x00ff < 0x100
#define CC_HOST_IS_BIG_ENDIAN (bool)(*(unsigned short *)"\0\xff" < 0x100) 
#define CC_SWAP32(i)  ((i & 0x000000ff) << 24 | (i & 0x0000ff00) << 8 | (i & 0x00ff0000) >> 8 | (i & 0xff000000) >> 24)
#define CC_SWAP16(i)  ((i & 0x00ff) << 8 | (i &0xff00) >> 8)   
#define CC_SWAP_INT32_LITTLE_TO_HOST(i) ((CC_HOST_IS_BIG_ENDIAN == true)? CC_SWAP32(i) : (i) )
#define CC_SWAP_INT16_LITTLE_TO_HOST(i) ((CC_HOST_IS_BIG_ENDIAN == true)? CC_SWAP16(i) : (i) )
#define CC_SWAP_INT32_BIG_TO_HOST(i)    ((CC_HOST_IS_BIG_ENDIAN == true)? (i) : CC_SWAP32(i) )
#define CC_SWAP_INT16_BIG_TO_HOST(i)    ((CC_HOST_IS_BIG_ENDIAN == true)? (i):  CC_SWAP16(i) )

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define COMM_IS_EMPTY(ptr) (!(ptr))

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif

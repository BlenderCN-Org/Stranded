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
	���ڴ�Ķ�̬�洢���з���n������Ϊsize�������ռ䣬��������һ��ָ�������ʼ��ַ��ָ�룻������䲻�ɹ�������NULL��
	��malloc������
		alloc�ڶ�̬�������ڴ���Զ���ʼ�����ڴ�ռ�Ϊ�㣬��malloc����ʼ�������������������������ݡ�
*/
#define COMM_CALLOC(p, n, s) \
	if (!((p) = calloc((n), (s)))) { \
		printf(stderr, "Insufficient memory!"); \
		exit(EXIT_FAILURE); \
	}

/*
	void *realloc(void *mem_address, unsigned int newsize);
	ָ���� =����������*��realloc��Ҫ�ı��ڴ��С��ָ�������µĴ�С����
	�µĴ�С�ɴ��С������Ҫע�⣬����µĴ�СС��ԭ�ڴ��С�����ܻᵼ�����ݶ�ʧ�����ã���

	���жϵ�ǰ��ָ���Ƿ����㹻�������ռ䣬����У�����mem_addressָ��ĵ�ַ�����ҽ�mem_address���أ�����ռ䲻����
	�Ȱ���newsizeָ���Ĵ�С����ռ䣬��ԭ�����ݴ�ͷ��β�������·�����ڴ����򣬶����ͷ�ԭ��mem_address��ָ�ڴ�����
	��ע�⣺ԭ��ָ�����Զ��ͷţ�����Ҫʹ��free����ͬʱ�����·�����ڴ�������׵�ַ�������·���洢����ĵ�ַ��
	����ֵ
	������·���ɹ��򷵻�ָ�򱻷����ڴ��ָ�룬���򷵻ؿ�ָ��NULL��
*/
#define COMM_REALLOC(p, s) \
	if (!((p) = realloc((p), (s)))) { \
		printf(stderr, "Insufficient memory!"); \
		exit(EXIT_FAILURE); \
	}

// ����-���б�������
#define COMM_SWAP(x, y, t) ((t) = (x), (x) = (y), (y) = (t))

// �Ƚ�����
#define  COMM_COMPARE(x, y) (((x) < (y)) ? -1 : ((x) == (y)) ? 0 : 1)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
// [2����]��С���ж�
int g_endian = 0x01020304;

ĿǰIntel��80x86ϵ��оƬ С�˴洢, �����ϴ��������ձ���õĶ��Ǵ��.

// �ڴ� ��λ->��λ
// С�˷�ʽ(��ַ����˳���ֵ����˳����ͬ�����ֽڸ�λ�洢): 04 03 02 01
// ��˷�ʽ(��ַ����˳���ֵ����˳���෴�����ֽڵ�λ�洢): 01 02 03 04

union
{
	int a;
	char b;
} endian;

// 1��ָ�뷽ʽ�ж�
char *p = (char*)&g_endian;
// *p == 4 С�� *p== 1 ���
printf("ret = [%d]\n", *p);
printf("&a = [%p]\n", &g_endian);

// 2�������巽ʽ�ж�
endian.a = g_endian;
printf("ret = [%d]\n", endian.b);
*/

// ��С��ת�� [��32λ����]
// 1���궨��
#define ENDIAN_TRANSFER(x) ( (((x)&0xff)<<24) | (((x)&0xff00)<<8) | (((x)&0xff0000)>>8) | (((x)&0xff000000)>>24) )
// 2������ʽ [�ַ�����ת�������ȼ�������λ]
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
////////////////////////cocos2dx�еķ���//////////////////////////////////////////////////
/**
Helper macros which converts 4-byte little/big endian
integral number to the machine native number representation

It should work same as apples CFSwapInt32LittleToHost(..)
*/

/// when define returns true it means that our architecture uses big endian
// С�ˣ� ��ַ��->�� 00 ff
// ʵ��ת�� unsigned short: 0xff00 = 65280
// > 0x100 �ڴ��ţ���ַ��->�� 00 01
///
// ��ˣ���ַ��->�� ff 00
// ʵ�� 0x00ff < 0x100
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

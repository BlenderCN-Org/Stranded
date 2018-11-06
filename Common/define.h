#ifndef __DEFINE_H__
#define __DEFINE_H__

#include <Windows.h>

#pragma warning(disable : 4996)	// unsafe warning
#pragma warning(disable : 4018) // warning C4018: “<”: 有符号/无符号不匹配


#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <math.h>
#include <tchar.h>

#include <string>
#include <vector>
#include <iostream>

#include <assert.h>

typedef struct Point
{
	double x, y, z;
} point;

#define MAX 400	//MAX为记录数组的最大数

typedef int datatype;	//定义关键字类型
typedef struct record	//定义记录为结构类型
{
	int key;	//记录的关键字域
	datatype other;	//记录的其他域
}rectype;

// 定义需要放置到cpp中，这里extern，避免重复包含
//rectype *s1, r[MAX];	//r[MAX]数组存放原始数据 *s1存放排序后的数据


#define MIN(a, b) (a < b) ? a : b

// 根据指针值删除内存
#ifndef SAFE_DELETE
#define SAFE_DELETE(x)	if((x) != NULL) { delete (x); (x)=NULL; }
#endif
// 根据指针值删除数组类型内存
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(x)	if((x) != NULL) { delete[] (x); (x)=NULL; }
#endif
// 根据指针调用free接口
#ifndef SAFE_FREE
#define SAFE_FREE(x)	if((x) != NULL) { free(x); (x)=NULL; }
#endif
// 根据指针调用Release接口
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)	if((x) != NULL) { (x)->Release(); (x)=NULL; }
#endif

//void (*pf) (int) throw(runtime_error);

// IP地址的字符最大长度
#define IP_SIZE		24
typedef USHORT		PacketID_t;
#define PACK_COMPART "$-$"	// 封包分隔符
#define PACK_COMPART_SIZE strlen(PACK_COMPART)

// 这样在可执行文件中将是汉字显示，发布版本要改成随机Key，Key不得小于10个字节
#define CLIENT_TO_LOGIN_KEY		"服务器端对客户端的封包密钥"
#define LOGIN_TO_CLIENT_KEY		"服务器端对客户端的封包密钥"

#ifndef ENCRYPT
#define ENCRYPT(x, xlen, KEY, BeginPlace)	if((x)!=NULL) \
	{\
		CHAR* pBuffer = (x); \
		CHAR* pKey = { KEY }; \
		UINT KeyLen = (UINT)strlen(pKey); \
		for (UINT i = 0; i < (xlen); i++) \
		{ \
			*pBuffer ^= pKey[(i + (BeginPlace)) % KeyLen]; \
			pBuffer++; \
		} \
	}
#endif

#ifndef ENCRYPT_HEAD
#define ENCRYPT_HEAD(x, KEY) if((x)!=NULL) \
	{ \
		CHAR* pBuffer = (x); \
		CHAR* pKey = { KEY }; \
		UINT KeyLen = (UINT)strlen(pKey); \
		for (UINT i = 0; i < PACKET_HEADER_SIZE; i++) \
		{ \
			*pBuffer ^= pKey[i%KeyLen]; \
			pBuffer++; \
		} \
	}
#endif

#endif
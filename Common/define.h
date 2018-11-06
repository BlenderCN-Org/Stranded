#ifndef __DEFINE_H__
#define __DEFINE_H__

#include <Windows.h>

#pragma warning(disable : 4996)	// unsafe warning
#pragma warning(disable : 4018) // warning C4018: ��<��: �з���/�޷��Ų�ƥ��


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

#define MAX 400	//MAXΪ��¼����������

typedef int datatype;	//����ؼ�������
typedef struct record	//�����¼Ϊ�ṹ����
{
	int key;	//��¼�Ĺؼ�����
	datatype other;	//��¼��������
}rectype;

// ������Ҫ���õ�cpp�У�����extern�������ظ�����
//rectype *s1, r[MAX];	//r[MAX]������ԭʼ���� *s1�������������


#define MIN(a, b) (a < b) ? a : b

// ����ָ��ֵɾ���ڴ�
#ifndef SAFE_DELETE
#define SAFE_DELETE(x)	if((x) != NULL) { delete (x); (x)=NULL; }
#endif
// ����ָ��ֵɾ�����������ڴ�
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(x)	if((x) != NULL) { delete[] (x); (x)=NULL; }
#endif
// ����ָ�����free�ӿ�
#ifndef SAFE_FREE
#define SAFE_FREE(x)	if((x) != NULL) { free(x); (x)=NULL; }
#endif
// ����ָ�����Release�ӿ�
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)	if((x) != NULL) { (x)->Release(); (x)=NULL; }
#endif

//void (*pf) (int) throw(runtime_error);

// IP��ַ���ַ���󳤶�
#define IP_SIZE		24
typedef USHORT		PacketID_t;
#define PACK_COMPART "$-$"	// ����ָ���
#define PACK_COMPART_SIZE strlen(PACK_COMPART)

// �����ڿ�ִ���ļ��н��Ǻ�����ʾ�������汾Ҫ�ĳ����Key��Key����С��10���ֽ�
#define CLIENT_TO_LOGIN_KEY		"�������˶Կͻ��˵ķ����Կ"
#define LOGIN_TO_CLIENT_KEY		"�������˶Կͻ��˵ķ����Կ"

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
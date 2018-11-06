#ifndef __C_DIRECT_MUSIC_H__
#define __C_DIRECT_MUSIC_H__

#define INITGUID

#include "defines.h"
#include "SoundInterface.h"

#include <windows.h>
#include <dmusicc.h>
#include <cguid.h>

#define STRANDED_INFINITE	-1

#if defined(D3D_SDK_VERSION) && (D3D_SDK_VERSION != 32)
#include <dmusici.h>

class CDMSoundObject : public CSoundInterface
{
public:
	CDMSoundObject();
	~CDMSoundObject() { Shutdown(); }

	bool Initialize(char * filename, int numRepeats);
	bool SetupSoundParameters(float dopplerFactor, float rolloffFactor, float minDist, float maxDist);

	void Play();
	void UpdateSoundPosition(float x, float y, float z);
	void Stop();
	void Shutdown();

private:
	// ���ּ�����
	IDirectMusicLoader8 *m_soundLoader;
	// ���ֲ���
	IDirectMusicPerformance8 *m_soundPerformance;
	// ��Ҫ������ʾ�����Ķ���
	IDirectMusicSegment8 *m_audioSound;
	// ��Ƶ·��
	IDirectMusicAudioPath *m_audioPath;
	// ������3D�ռ��в�������
	IDirectSound3DBuffer *m_audioBuffer;
	// ͬ�����ڲ���3D ����
	IDirectSound3DListener *m_audioListener;
	
	/////////////////////// ���ڻ�ȡ������ 3D ������������������� ////////////////////////////////////////
	DS3DBUFFER m_bufferParams;
	DS3DLISTENER m_listenerParams;
};

//////////////////////////////////////////////////////////////////////////

/*
 * �洢�ڲ��������������������ٸ����������е������������Ƿ�Ҫ��ʼ�� COM.
 * id - ��Ҫʹ�õ��������������һ������
 */
class CDirectMusicSystem : public CSoundSystemInterface
{
public:
	CDirectMusicSystem();
	~CDirectMusicSystem() { Shutdown(); }

	bool AddSound(char *soundfile, int numRepeats, int *id);
	bool SetupSoundParameters(int id, float dopplerFactor, float rolloffFactor, float minDist, float maxDist);

	void Play(int id);
	void UpdateSoundPosition(int id, float x, float y, float z);
	void Stop(int id);

	void Shutdown();

private:
	int IncreaseSounds();

private:
	bool m_comInit;
	int m_totalSounds;
	CDMSoundObject * m_soundList;
};

#else
class CDirectMusicSystem : public CSoundSystemInterface
{
public:
	CDirectMusicSystem() {};
	~CDirectMusicSystem() { Shutdown(); }

	bool AddSound(char *soundfile, int numRepeats, int *id) { return false; };
	bool SetupSoundParameters(int id, float dopplerFactor, float rolloffFactor, float minDist, float maxDist) { return false; };

	void Play(int id) {};
	void UpdateSoundPosition(int id, float x, float y, float z) {};
	void Stop(int id) {};

	void Shutdown() {};

private:
	bool m_comInit;
	int m_totalSounds;
};
#endif

bool CreateDMSound(CSoundSystemInterface **pObj);

#endif

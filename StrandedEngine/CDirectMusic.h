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
	// 音乐加载器
	IDirectMusicLoader8 *m_soundLoader;
	// 音乐播放
	IDirectMusicPerformance8 *m_soundPerformance;
	// 主要用来表示声音的对象
	IDirectMusicSegment8 *m_audioSound;
	// 音频路径
	IDirectMusicAudioPath *m_audioPath;
	// 用于在3D空间中播放声音
	IDirectSound3DBuffer *m_audioBuffer;
	// 同样用于播放3D 声音
	IDirectSound3DListener *m_audioListener;
	
	/////////////////////// 用于获取和设置 3D 声音缓存和侦听器对象 ////////////////////////////////////////
	DS3DBUFFER m_bufferParams;
	DS3DLISTENER m_listenerParams;
};

//////////////////////////////////////////////////////////////////////////

/*
 * 存储内部声音对象数组链表，跟踪该链表，链表中的声音总数，是否要初始化 COM.
 * id - 想要使用的声音对象链表的一个索引
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

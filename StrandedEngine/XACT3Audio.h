#ifndef __XACT3_AUDIO_H__
#define __XACT3_AUDIO_H__

/*
* XACT3:
* the high-level audio API
*
* @example:
CXACT3Audio::GetInstance()->Init("music\\Win\\XactAudioProject.xgs");
CXACT3Audio::GetInstance()->LoadWaveBank("music\\Win\\Wave Bank.xwb");
CXACT3Audio::GetInstance()->LoadSoundBank("music\\Win\\Sound Bank.xsb");

CSoundCue music = CXACT3Audio::GetInstance()->GetSoundCue("music");
music.Play();
music.Stop();

// This is how to play a sound cue safely from a button press
if( CGameController::Instance().WasAction("music") )
{
	if( !music.IsBusy() )
	{
		music.Play();
	}
}

void CGame::Update()
{
	// This is how you can play sound at a point in 3D space.
	// This is not something you need to do. Just showing you it's available
	if (music.IsBusy())
	{
		camera.Finialize();
		CPoint point = camera.GetFinialMatrix().GetMatrixPoint();
		point.Invert();
		CXACT3Audio::GetInstance()->PositionCue(point, music.pCue);
	}

	// This needs to be called every game loop to keep sound streaming happy
	CXACT3Audio::GetInstance()->Update();

}	// Update
*/

#include <xact3.h>
#include <xact3d3.h>

#include <map>

#include "soundcue.h"
#include "xactdefs.h"
#include "vector.h"

class CXACT3Audio
{
public:
	// Get the class instance
	static CXACT3Audio* GetInstance();
	static void DestroyInstance();

private:
	// a singleton
	CXACT3Audio();
	virtual ~CXACT3Audio();

public:
	// Init the xact audio system with the global settings file
	bool Init(const char* filePath);

	// Load a wave bank
	bool LoadWaveBank(const char* filePath);

	// Destory a wave bank
	void DestoryWaveBank(const char* filePath);

	// Load a sound bank
	bool LoadSoundBank(const char* filePath);

	// Destory a sound bank
	void DestorySoundBank(const char* filePath);

	// Allows XACT to do required periodic work. call often
	virtual void Update();

	// Get a sound cue
	CSoundCue GetSoundCue(const char* sCueStr);

	// Play a sound
	void Play(const char* sCueStr);

	// Stop a sound
	void Stop(const char* sCueStr);

	// Prepare a sound
	void Prepare(const char* sCueStr);

	// Position the sound cue based on the point
	void PositionCue(CVector3& point, IXACT3Cue* pCue);

	// 清理数据
	void ShutDown();

protected:
	// notification call back function - needs to be static to be passed as function pointer
	static void WINAPI XACTNotificationCallback(const XACT_NOTIFICATION* pNotification);

	// Function for handling XACT notifications
	virtual void HandleNotification(const XACT_NOTIFICATION* pNotification);

	// Display error information
	void DisplayError(HRESULT hr);

	// Init the class member variables for 3D sound
	void Init3DSound();

protected:
	// a ULONG used to signal when entering and exiting a critical state
	CRITICAL_SECTION m_criticalSection;

	// XACT audio engine
	IXACT3Engine* m_pEngine;

	// 3D audio instance
	X3DAUDIO_HANDLE m_x3DInstance;

	// map for wave bank
	std::map<std::string, CWaveBank*> m_waveBankMap;

	// map for sound bank
	std::map<std::string, CSoundBank*> m_soundBankMap;

	// 3D sound data members
	X3DAUDIO_DSP_SETTINGS m_dspSettings;
	X3DAUDIO_LISTENER m_listener;
	X3DAUDIO_EMITTER m_emitter;
	FLOAT32 m_delayTimes[2];
	FLOAT32 m_matrixCoefficients[2 * 8];
};

#endif // !__XACT3_AUDIO_H__


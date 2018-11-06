#ifndef __SOUND_CUE_H__
#define __SOUND_CUE_H__

#include <xact3.h>

class CSoundCue
{
public:
	CSoundCue() : m_pSoundBank(NULL), m_curIndex(XACTINDEX_INVALID), m_pCue(NULL) {}
	CSoundCue(const CSoundCue& obj)
	{
		*this = obj;
	}

	~CSoundCue() {}

public:
	// Play a sound
	bool Play()
	{
		if ((m_pSoundBank != NULL) && (m_curIndex != XACTINDEX_INVALID))
		{
			/*
			 * play the cue by index
			 * HRESULT PLAY(XACTINDEX nCueIndex, DWORD dwFlags, XACTTIME timeOffset, IXACT3Cue** ppCue);
			 * @param: the cue index、the playback fags(unused)、 the offset in milliseconds to start the playback、the address to an IXACT3Cue object that stores the cue being played(optional).
			 */
			if (FAILED(m_pSoundBank->Play(m_curIndex, 0, 0, &m_pCue)))
				return false;

			return true;
		}

		return false;
	}

	// stop a sound
	bool Stop()
	{
		if ((m_pSoundBank != NULL) && (m_curIndex != XACTINDEX_INVALID))
		{
			/*
			 * stop a cue that is being played
			 * @param: the cue index、flag(XACT_FLAG_SOUNDBANK_STOP_IMMEDIATE)
			 * HRESULT Stop(XACTINDEX nCueIndex, DWORD dwFlags);
			 */
			if (FAILED(m_pSoundBank->Stop(m_curIndex, 0)))
				return false;

			return true;
		}

		return false;
	}

	// Prepare a sound
	bool Prepare()
	{
		if ((m_pSoundBank != NULL) && (m_curIndex != XACTINDEX_INVALID))
		{
			if (FAILED(m_pSoundBank->Prepare(m_curIndex, 0, 0, &m_pCue)))
				return false;

			return true;
		}

		return false;
	}

	/*
	 * @brief: Check if a cue is busy.
	 * @param:
	 * @return: bool - true is busy.
	 */
	bool IsBusy()
	{
		DWORD cueState = GetState();

		return ((cueState > XACT_STATE_CREATED) && (cueState < XACT_STATE_STOPPED));
	}

	/*
	 * @brief: Get the state of the sound.
	 * @param:
	 * @return: DWORD - state value of cue
	 */
	DWORD GetState()
	{
		DWORD cueState(0);

		if (m_pCue != NULL)
		{
			if (FAILED(m_pCue->GetState(&cueState)))
				cueState = 0;
		}

		return cueState;
	}
	
public:
	// Pointer to sound bank
	IXACT3SoundBank* m_pSoundBank;

	// Index of sound cue
	XACTINDEX m_curIndex;

	// Cue instance pointer
	IXACT3Cue* m_pCue;
};

#endif

#ifndef __XACT_DEFS_H__
#define __XACT_DEFS_H__

#include <xact3.h>

struct CWaveBankFileHeader
{
	char header[4];
	int version;
	int headerSize1;
	int headerSize2;
	int offsetDetailsDir;
	int lengthDetailsDir;
	int offsetFileNameDir;
	int lengthFileNameDir;
	int firstFileOffset;
	int unknown1;
	short int unknown2[6];
	short int streaming;
	short int unknown3;
	int noOfFiles;
	char fileName[16];
	int lengthDetailEntry;
	int lengthFilenameEntry;
	int paddingBetweenFiles;
	int nullValue;
};

struct CWaveBank
{
	CWaveBank() : m_pWaveBank(NULL), m_pMemMapBuffer(NULL), m_streamFileHandle(NULL) {}

	// pointer to wav bank
	IXACT3WaveBank* m_pWaveBank;

	// pointer to memory mapped buffer
	VOID* m_pMemMapBuffer;

	// handle to stream file
	HANDLE m_streamFileHandle;
};

struct CSoundBank
{
	CSoundBank() : m_pSoundBank(NULL), m_PSoundBankBuffer(NULL) {}
	
	// Pointer to sound bank
	IXACT3SoundBank* m_pSoundBank;

	// Allocated sound bank buffer
	unsigned char* m_PSoundBankBuffer;
};

#endif

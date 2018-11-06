#include "XACT3Audio.h"

#include "deletefuncs.h"
#include <d3dx9.h>

#ifdef _MSC_VER
#pragma comment(lib, "X3DAudio.lib")
#endif


//////////////////////////////////////////////////////////////////////////

static CXACT3Audio *sharedXACT3Audio = nullptr;

CXACT3Audio* CXACT3Audio::GetInstance()
{
	if (!sharedXACT3Audio)
	{
		sharedXACT3Audio = new CXACT3Audio;
	}

	return sharedXACT3Audio;
}

void CXACT3Audio::DestroyInstance()
{
	if (sharedXACT3Audio)
	{
		delete sharedXACT3Audio;
		sharedXACT3Audio = nullptr;
	}
}

CXACT3Audio::CXACT3Audio() : m_pEngine(NULL)
{

}

CXACT3Audio::~CXACT3Audio()
{
	ShutDown();
}

/*
 * @brief: Init the xact audio system with the global settings file.
 *			the global setting file has to be loaded first.
 * @param: path to the global settings file (.xgs)
 * @return: bool - false on fail
 */
bool CXACT3Audio::Init(const char * filePath)
{
	HRESULT hr;

	// Set critical section variable
	::InitializeCriticalSection(&m_criticalSection);

	// initializes COM with a call to ConInitializeEx, which is necessary  for XACT because it prepares a thread to use the COM library.
	if (SUCCEEDED(hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED)))
	{
		/*
		* create the audio engine
		* HRESULT XACT3CreateEngine(DWORD dwCreationFlags, IXACT3Engine** ppEngine);
		* param:
		the creation flag for the audio engine. XACT_FLAG_API_AUDITION_MODE (which is ignored on the XBOX 360); XACT_FLAG_API_DEBUG_MODE (debug mode)
		the address of a pointer that will store the returned audio engine.
		*/
		hr = XACT3CreateEngine(XACT_FLAG_API_AUDITION_MODE, &m_pEngine);
	}

	if (FAILED(hr) || m_pEngine == NULL)
	{
		DisplayError(hr);
		return false;
	}

	bool bSucess = false;

	//////////////////////////////////////////////////////////////////////////
	// Initialize & create the XACT runtime
	XACT_RUNTIME_PARAMETERS xrParams = { 0 };
	xrParams.globalSettingsFlags = XACT_FLAG_GLOBAL_SETTINGS_MANAGEDATA;
	xrParams.fnNotificationCallback = XACTNotificationCallback;
	// the look-ahead time (time XACT reads ahead in milliseconds).
	xrParams.lookAheadTime = XACT_ENGINE_LOOKAHEAD_DEFAULT;

	HANDLE hFile = ::CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != NULL)
	{
		xrParams.globalSettingsBufferSize = ::GetFileSize(hFile, NULL);
		if (xrParams.globalSettingsBufferSize != INVALID_FILE_SIZE)
		{
			// Using CoTaskMemAlloc so that XACT can clean up this data when it's done.
			xrParams.pGlobalSettingsBuffer = ::CoTaskMemAlloc(xrParams.globalSettingsBufferSize);
			if (xrParams.pGlobalSettingsBuffer != NULL)
			{
				DWORD dwBytesRead;
				if (0 != ::ReadFile(hFile, xrParams.pGlobalSettingsBuffer, xrParams.globalSettingsBufferSize, &dwBytesRead, NULL))
					bSucess = true;
			}
		}

		::CloseHandle(hFile);
	}

	if (!bSucess)
	{
		if (xrParams.pGlobalSettingsBuffer != NULL)
		{
			::CoTaskMemFree(xrParams.pGlobalSettingsBuffer);
		}

		::OutputDebugString("\n[XACT Error]: Error reading in global settings file.\n");
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	/*
	* initialize the audio engine
	* HREAULT Initialize(const XACT_RUNTIME_PARAMETERS* pParams);
	*/
	if (FAILED(hr = m_pEngine->Initialize(&xrParams)))
	{
		DisplayError(hr);
		return false;
	}

	// Initialize 3D settings
	if (FAILED(hr = XACT3DInitialize(m_pEngine, m_x3DInstance)))
	{
		DisplayError(hr);
		return false;
	}

	// Init for 3D sound
	Init3DSound();

	return true;
}

/*
 * @brief: Load a wave bank
 * @param: filePath - path to the wave bank file (.xwb)
 * @return: bool - false of fail
 */
bool CXACT3Audio::LoadWaveBank(const char* filePath)
{
	CWaveBankFileHeader header;
	unsigned long bytesRead;
	DWORD dwFileSize;
	HRESULT hr;
	CWaveBank* pWaveBank;

	// See if this wave bank has already been
	std::map<std::string, CWaveBank*>::iterator iter = m_waveBankMap.find(filePath);
	// If it's not found, allocate the wave bank class and add it to the list
	if (iter == m_waveBankMap.end())
	{
		pWaveBank = new CWaveBank;
		m_waveBankMap.insert(std::make_pair(filePath, pWaveBank));
	}
	else
	{
		OutputDebugString("\n[Error] Wave bank already loaded.\n");
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// pats of the xwb file header is unknown. the flag that indicates if the wav bank is loaded into memory or that it is ment to be streamed.
	//////////////////////////////////////////////////////////////////////////

	// Open the file and read in the header
	HANDLE hFile = ::CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	// Check to see that we have a valid file handle
	if (hFile == INVALID_HANDLE_VALUE)
	{
		OutputDebugString("\n[Error] Can't open wave bank to read header information.\n");
		::CloseHandle(hFile);
		return false;
	}

	// Read contents of file into memory.
	BOOL readResult = ::ReadFile(hFile, &header, sizeof(header), &bytesRead, NULL);
	if (!readResult || bytesRead != sizeof(header))
	{
		OutputDebugString("\n[Error] Can't read wave bank for header information.\n");
		::CloseHandle(hFile);
		return false;
	}

	::CloseHandle(hFile);

	if (header.streaming)
	{
		hFile = ::CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			/*
			 *
				typedef struct XACT_STREAMING_PARAMETERS {
				HANDLE file;		// a handle to the wave bank file
				DWORD offset;		// the offset in bytes into the wave bank file and must be DVD sector aligned.
				DWORD flags;		// the streaming flag, which is currently unused and should be set to 0.
				WORD packetSize;	// the packet size, which dictates the stream packet size (in sectors) to use for each data stream.
				} XACT_STREAMING_PARAMETERS, *LPXACT_STREAMING_PARAMETERS,
				XACT_WAVEBANK_STREAMING_PARAMETERS, *LPXACT_WAVEBANK_STREAMING_PARAMETERS;
			 */
			XACT_WAVEBANK_STREAMING_PARAMETERS wsParams;
			ZeroMemory(&wsParams, sizeof(XACT_WAVEBANK_STREAMING_PARAMETERS));
			wsParams.file = hFile;
			wsParams.offset = 0;

			// 64 means to allocate a 64 * 2k buffer for streaming.
			// This is a good size for DVD streaming and takes good advantage of the read ahead cache
			wsParams.packetSize = 64;

			/*
			 *
				HRESULT CreateStreamingWaveBank(const XACT_WAVEBANK_STREAMING_PARAMETERS* pParams, IXACT3WaveBank** ppWaveBank);
			 */
			if (FAILED(hr = m_pEngine->CreateStreamingWaveBank(&wsParams, &pWaveBank->m_pWaveBank)))
			{
				DisplayError(hr);
				OutputDebugString("\n[Error] Can't create streaming wave bank.\n");
				return false;
			}
		}
		else
		{
			OutputDebugString("\n[Error] Can't open wave bank to create streaming wave bank.\n");
			::CloseHandle(hFile);
			return false;
		}

		// Record the file handle
		pWaveBank->m_streamFileHandle = hFile;
	}
	else
	{
		hr = E_FAIL;

		// Read and register the wave bank file with XACT using memory mapped file IO.
		// Memory mapped files tend to be the fastest for most situations assuming you have enough virtual address space for a full map of file
		hFile = ::CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			dwFileSize = ::GetFileSize(hFile, NULL);
			if (dwFileSize != -1)
			{
				HANDLE hMapFile = ::CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, dwFileSize, NULL);
				if (hMapFile)
				{
					// obtain a ponter to the buffer
					pWaveBank->m_pMemMapBuffer = ::MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
					if (pWaveBank->m_pMemMapBuffer != NULL)
					{
						/*
						 *
							HRESULT CreateInMemoryWaveBank(
							const void* pvBuffer,		// a pointer to the buffer of data that is the wave bank file. (.xwb)
							DWORD dwSize,				// the size of the audio buffer in bytes.
							DWORD dwFlags,				// the creation flag, which can be 0 or XACT_FLAG_ENGINE_CREATE_MANAGEDATA ( XACT3 will free the buffer upon the wave bank's release)
							DWORD dwAllocAttributes,	// only used by the XBOX 360
							IXACT3WaveBank** ppWaveBank	// the address of the wave bank
							)
						 */
						hr = m_pEngine->CreateInMemoryWaveBank(pWaveBank->m_pMemMapBuffer, dwFileSize, 0, 0, &pWaveBank->m_pWaveBank);
					}
					::CloseHandle(hMapFile);	// pbWaveBank is maintains a handle on the file so close this unneeded handle
				}
			}
			::CloseHandle(hFile);
		}

		if (FAILED(hr))
		{
			DisplayError(hr);
			OutputDebugString("\n[Error] Can't create in memory wave bank.");
			return false;
		}
	}

	// Register for XACT notification if this wave bank is to be destroyed
	XACT_NOTIFICATION_DESCRIPTION desc = { 0 };
	desc.flags = XACT_FLAG_NOTIFICATION_PERSIST;
	desc.type = XACTNOTIFICATIONTYPE_WAVEBANKDESTROYED;
	desc.pWaveBank = pWaveBank->m_pWaveBank;
	desc.pvContext = this;

	if (FAILED(hr = m_pEngine->RegisterNotification(&desc)))
	{
		DisplayError(hr);
		return false;
	}

	return true;
}

void CXACT3Audio::DestoryWaveBank(const char * filePath)
{
	// See if this wave bank has been loaded
	std::map<std::string, CWaveBank*>::iterator iter = m_waveBankMap.find(filePath);
	// Only destroy it if it has been found
	if (iter != m_waveBankMap.end())
	{
		// an XACTNotificationCallback will be sent and there is wheres the clean-up will be handled
		iter->second->m_pWaveBank->Destroy();
	}
}

/*
 * @brief: Load a sound bank
 * @param: filePath - path to the sound bank file
 * @return: bool - false of fail
 */
bool CXACT3Audio::LoadSoundBank(const char * filePath)
{
	unsigned long bytesRead;
	DWORD dwFileSize;
	HRESULT hr;
	CSoundBank* pSoundBank;

	// See if this wave bank has already loaded
	std::map<std::string, CSoundBank*>::iterator iter = m_soundBankMap.find(filePath);
	// If it's not found, allocate the wave bank class and add it to the list
	if (iter == m_soundBankMap.end())
	{
		pSoundBank = new CSoundBank;
		m_soundBankMap.insert(std::make_pair(filePath, pSoundBank));
	}
	else
	{
		OutputDebugString("\n[Error] Sound bank already loaded.\n");
		return false;
	}

	hr = E_FAIL;
	HANDLE hFile = ::CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		dwFileSize = ::GetFileSize(hFile, NULL);
		if (dwFileSize != -1)
		{
			// Allocate the data here and free the data when recieving the sound bank destroyed notification
			pSoundBank->m_PSoundBankBuffer = new unsigned char[dwFileSize];
			if (pSoundBank->m_PSoundBankBuffer != NULL)
			{
				BOOL readResult = ::ReadFile(hFile, pSoundBank->m_PSoundBankBuffer, dwFileSize, &bytesRead, NULL);
				if (readResult || bytesRead == dwFileSize)
				{
					/*
					 *
						HRESULT CreateSoundBank(
						const void* pvBuffer,			// the file data(buffer) of the sound bank
						DWORD dwSize,					// the size of the data buffer in bytes
						DWORD dwFlags,					// the creation flag, which can be 0 or XACT_FLAG_ENGINE_CREATE_MANAGEDATA ( XACT3 will free the buffer upon the wave bank's release)
						DWORD dwAllocAttributes,		// only used by the XBOX 360
						IXACT3SoundBank** ppSoundBank	// the address of the sound bank
						);
					 */
					hr = m_pEngine->CreateSoundBank(pSoundBank->m_PSoundBankBuffer, dwFileSize, 0, 0, &pSoundBank->m_pSoundBank);
				}
			}
		}
		::CloseHandle(hFile);
	}

	if (FAILED(hr))
	{
		DisplayError(hr);
		OutputDebugString("\n[Error] Can't create sound bank.\n");
		return false;
	}

	// Register for XACT notification if this wave bank is to be destroyed.
	XACT_NOTIFICATION_DESCRIPTION desc = { 0 };
	desc.flags = XACT_FLAG_NOTIFICATION_PERSIST;
	desc.type = XACTNOTIFICATIONTYPE_SOUNDBANKDESTROYED;
	desc.pSoundBank = pSoundBank->m_pSoundBank;
	desc.pvContext = this;

	if (FAILED(hr = m_pEngine->RegisterNotification(&desc)))
	{
		DisplayError(hr);
		return false;
	}

	return true;
}

void CXACT3Audio::DestorySoundBank(const char * filePath)
{
	// See if this sound bank has been loaded
	std::map<std::string, CSoundBank*>::iterator iter = m_soundBankMap.find(filePath);
	// Only destroy it if it has been found
	if (iter != m_soundBankMap.end())
	{
		// an XACTNotificationCallback will be sent and there is wheres the clean-up will be handled
		iter->second->m_pSoundBank->Destroy();
	}
}

/*
 * @brief: Allows XACT to do required periodic work. call within game loop
 */
void CXACT3Audio::Update()
{
	if (m_pEngine != NULL)
	{
		/*
		 * during the application's execution, and every so often, the audio engine will need to update itself.
		 * Ideally, calling DoWork could occur as frequently as once per frame as long as it is done periodically.
		 * HRESULT DoWork();
		 */
		m_pEngine->DoWork();
	}
}

CSoundCue CXACT3Audio::GetSoundCue(const char * sCueStr)
{
	CSoundCue soundCue;

	for (std::map<std::string, CSoundBank*>::iterator iter = m_soundBankMap.begin(); iter != m_soundBankMap.end(); iter++)
	{
		/*
		 * @param: a null-terminated string that represents the the audio cue, which have named in the XACT GUI tool when creating the cue
		 * XACTINDEX GetCueIndex(PCSTR szFriendlyName);
		 * access sound cues, obtain the index of a cue.
		 */
		if (XACTINDEX_INVALID != (soundCue.m_curIndex = iter->second->m_pSoundBank->GetCueIndex(sCueStr)))
		{
			// copy the pointer to the sound bank
			soundCue.m_pSoundBank = iter->second->m_pSoundBank;
			break;
		}
	}

	return soundCue;
}

/*
 * @brief: Play a sound
 * @param: sCueStr - string ID of sound cue. Name entered in XACT tool.
 */
void CXACT3Audio::Play(const char * sCueStr)
{
	GetSoundCue(sCueStr).Play();
}

/*
* @brief: Stop a sound
* @param: sCueStr - string ID of sound cue. Name entered in XACT tool.
*/
void CXACT3Audio::Stop(const char * sCueStr)
{
	GetSoundCue(sCueStr).Stop();
}

/*
* @brief: Prepare a sound
* @param: sCueStr - string ID of sound cue. Name entered in XACT tool.
*/
void CXACT3Audio::Prepare(const char* sCueStr)
{
	GetSoundCue(sCueStr).Prepare();
}

/*
 * @brief: Position the sound cue based on the point
 */
void CXACT3Audio::PositionCue(CVector3 & point, IXACT3Cue * pCue)
{
	m_listener.Position = D3DXVECTOR3(point.x, point.y, point.z);

	XACT3DCalculate(m_x3DInstance, &m_listener, &m_emitter, &m_dspSettings);
	XACT3DApply(&m_dspSettings, pCue);
}

void CXACT3Audio::ShutDown()
{
	for (std::map<std::string, CSoundBank*>::iterator iter = m_soundBankMap.begin(); iter != m_soundBankMap.end(); iter++)
	{
		// The Destroy function of the banks stops all audio playbacks immediately and releases all resources used by them.
		if (iter->second->m_pSoundBank)
			iter->second->m_pSoundBank->Destroy();

		// Delete the sound bank buffers
		DeleteArray(iter->second->m_PSoundBankBuffer);
	}

	DeleteMapPointers(m_soundBankMap);

	for (std::map<std::string, CWaveBank*>::iterator iter = m_waveBankMap.begin(); iter != m_waveBankMap.end(); iter++)
	{
		if (iter->second->m_pWaveBank)
			iter->second->m_pWaveBank->Destroy();

		if (iter->second->m_pMemMapBuffer != NULL)
		{
			::UnmapViewOfFile(iter->second->m_pMemMapBuffer);
			iter->second->m_pMemMapBuffer = NULL;
		}
		else if (iter->second->m_streamFileHandle != NULL)
		{
			::CloseHandle(iter->second->m_streamFileHandle);
			iter->second->m_streamFileHandle = NULL;
		}
	}

	DeleteMapPointers(m_waveBankMap);

	::CoUninitialize();
	
	::DeleteCriticalSection(&m_criticalSection);
}

/*
 * \brief: This is the callback for handling XACT notifications.
 * \params: pNotification- notifications pointer.
 */
void CXACT3Audio::XACTNotificationCallback(const XACT_NOTIFICATION * pNotification)
{
	if ((pNotification != NULL) && (pNotification->pvContext != NULL))
	{
		// Cast pointer. Be sure to set pNotification->pvContext to the "this" pointer
		CXACT3Audio* pXAct = reinterpret_cast<CXACT3Audio*>(pNotification->pvContext);
		pXAct->HandleNotification(pNotification);
	}
}

/*
 * @brief: Function for handling XACT notifications
 */
void CXACT3Audio::HandleNotification(const XACT_NOTIFICATION * pNotification)
{
	if (pNotification->type == XACTNOTIFICATIONTYPE_WAVEBANKDESTROYED)
	{
		// Close memory map and streaming file handles
		for (std::map<std::string, CWaveBank*>::iterator iter = m_waveBankMap.begin(); iter != m_waveBankMap.end(); iter++)
		{
			// Find the wave bank we want to delete
			if (iter->second->m_pWaveBank == pNotification->waveBank.pWaveBank)
			{
				if (iter->second->m_pMemMapBuffer != NULL)
				{
					::UnmapViewOfFile(iter->second->m_pMemMapBuffer);
					iter->second->m_pMemMapBuffer = NULL;
				}
				else if (iter->second->m_streamFileHandle != NULL)
				{
					::CloseHandle(iter->second->m_streamFileHandle);
					iter->second->m_streamFileHandle = NULL;
				}
				Delete(iter->second);
				OutputDebugString("\n[Debug] Wave Bank Delete.\n");
				m_waveBankMap.erase(iter);
				break;
			}
		}
	}
	else if (pNotification->type == XACTNOTIFICATIONTYPE_SOUNDBANKDESTROYED)
	{
		for (std::map<std::string, CSoundBank*>::iterator iter = m_soundBankMap.begin(); iter != m_soundBankMap.end(); iter++)
		{
			// Find the sound bank we want to delete
			if (iter->second->m_pSoundBank == pNotification->soundBank.pSoundBank)
			{
				DeleteArray(iter->second->m_PSoundBankBuffer);
				Delete(iter->second);
				OutputDebugString("\n[Debug] Sound Bank Delete.\n");
				m_soundBankMap.erase(iter);
				break;
			}
		}
	}
}

/*
 * @brief: Display error information.
 * @param: hr - return result from function call.
 */
void CXACT3Audio::DisplayError(HRESULT hr)
{
	switch (hr)
	{
	case XACTENGINE_E_OUTOFMEMORY:
		OutputDebugString("\n[Error] XACT engine out of memory\n");
		break;
	case XACTENGINE_E_INVALIDARG:
		OutputDebugString("\n[Error] Invalid arguments\n");
		break;
	case XACTENGINE_E_NOTIMPL:
		OutputDebugString("\n[Error] Feature not implemented\n");
		break;
	case XACTENGINE_E_ALREADYINITIALIZED:
		OutputDebugString("\n[Error] XACT engine is already initialized.\n");
		break;
	case XACTENGINE_E_NOTINITIALIZED:
		OutputDebugString("\n[Error] XACT engine has not been initialized.\n");
		break;
	case XACTENGINE_E_EXPIRED:
		OutputDebugString("\n[Error] XACT engine has expired (demo or pre-release version).\n");
		break;
	case XACTENGINE_E_NONOTIFICATIONCALLBACK:
		OutputDebugString("\n[Error] No notification callback.\n");
		break;
	case XACTENGINE_E_NOTIFICATIONREGISTERED:
		OutputDebugString("\n[Error] Notification already registered.\n");
		break;
	case XACTENGINE_E_INVALIDUSAGE:
		OutputDebugString("\n[Error] Invalid usage.\n");
		break;
	case XACTENGINE_E_INVALIDDATA:
		OutputDebugString("\n[Error] Invalid data.\n");
		break;
	case XACTENGINE_E_INSTANCELIMITFAILTOPLAY:
		OutputDebugString("\n[Error] Fail to play due to instance limit.\n");
		break;
	case XACTENGINE_E_NOGLOBALSETTINGS:
		OutputDebugString("\n[Error] Global Settings not loaded.\n");
		break;
	case XACTENGINE_E_INVALIDVARIABLEINDEX:
		OutputDebugString("\n[Error] Invalid variable index.\n");
		break;
	case XACTENGINE_E_INVALIDCATEGORY:
		OutputDebugString("\n[Error] Invalid category.\n");
		break;
	case XACTENGINE_E_INVALIDCUEINDEX:
		OutputDebugString("\n[Error] Invalid cue index.\n");
		break;
	case XACTENGINE_E_INVALIDWAVEINDEX:
		OutputDebugString("\n[Error] Invalid wave index.\n");
		break;
	case XACTENGINE_E_INVALIDTRACKINDEX:
		OutputDebugString("\n[Error] Invalid track index.\n");
		break;
	case XACTENGINE_E_INVALIDSOUNDOFFSETORINDEX:
		OutputDebugString("\n[Error] Invalid sound offset or index.\n");
		break;
	case XACTENGINE_E_READFILE:
		OutputDebugString("\n[Error] Error reading a file.\n");
		break;
	case XACTENGINE_E_UNKNOWNEVENT:
		OutputDebugString("\n[Error] Unknown event type.\n");
		break;
	case XACTENGINE_E_INCALLBACK:
		OutputDebugString("\n[Error] Invalid call of method function from callback.\n");
		break;
	case XACTENGINE_E_NOWAVEBANK:
		OutputDebugString("\n[Error] No wavebank exists for desired operation.\n");
		break;
	case XACTENGINE_E_SELECTVARIATION:
		OutputDebugString("\n[Error] Unable to select a variation.\n");
		break;
	case XACTENGINE_E_MULTIPLEAUDITIONENGINES:
		OutputDebugString("\n[Error] There can be only one audition engine.\n");
		break;
	case XACTENGINE_E_WAVEBANKNOTPREPARED:
		OutputDebugString("\n[Error] The wavebank is not prepared.\n");
		break;
	case XACTENGINE_E_NORENDERER:
		OutputDebugString("\n[Error] No audio device found on.\n");
		break;
	case XACTENGINE_E_INVALIDENTRYCOUNT:
		OutputDebugString("\n[Error] Invalid entry count for channel maps.\n");
		break;
	case XACTENGINE_E_SEEKTIMEBEYONDCUEEND:
		OutputDebugString("\n[Error] Time offset for seeking is beyond the cue end.\n");
		break;
	case XACTENGINE_E_SEEKTIMEBEYONDWAVEEND:
		OutputDebugString("\n[Error] Time offset for seeking is beyond the wave end.\n");
		break;
	case XACTENGINE_E_NOFRIENDLYNAMES:
		OutputDebugString("\n[Error] Friendly names are not included in the bank.\n");
		break;
	default:
		OutputDebugString("\n[Error] Unknown error.\n");
		break;
	}
}

void CXACT3Audio::Init3DSound()
{
	HRESULT hr;

	// Setup 3D audio structs
	ZeroMemory(&m_listener, sizeof(m_listener));
	m_listener.OrientFront = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	m_listener.OrientTop = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	m_listener.Position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_listener.Velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	ZeroMemory(&m_emitter, sizeof(m_emitter));
	m_emitter.pCone = NULL;
	m_emitter.OrientFront = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	m_emitter.OrientTop = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	m_emitter.Position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_emitter.Velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_emitter.ChannelCount = 2;
	m_emitter.ChannelRadius = 1.0f;
	m_emitter.pChannelAzimuths = NULL;
	m_emitter.pVolumeCurve = NULL;
	m_emitter.pLFECurve = NULL;
	m_emitter.pLPFDirectCurve = NULL;
	m_emitter.pLPFReverbCurve = NULL;
	m_emitter.pReverbCurve = NULL;	// »ìÏìÇúÏß
	m_emitter.CurveDistanceScaler = 1.0f;
	m_emitter.DopplerScaler = NULL;

	// query number of channels on the final mix
	WAVEFORMATEXTENSIBLE wfxFinalMixFormat;
	if (FAILED(hr = m_pEngine->GetFinalMixFormat(&wfxFinalMixFormat)))
	{
		// Just show an error message. Don't kill the game over this
		DisplayError(hr);
	}

	// Init MatrixCoefficients. XACT will fill in the values
	ZeroMemory(&m_matrixCoefficients, sizeof(m_matrixCoefficients));

	ZeroMemory(&m_delayTimes, sizeof(m_delayTimes));
	ZeroMemory(&m_dspSettings, sizeof(m_dspSettings));
	m_dspSettings.pMatrixCoefficients = m_matrixCoefficients;
	m_dspSettings.pDelayTimes = m_delayTimes;
	m_dspSettings.SrcChannelCount = 2;
	m_dspSettings.DstChannelCount = wfxFinalMixFormat.Format.nChannels;
}

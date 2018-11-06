#include "CDirectMusic.h"

#if defined(D3D_SDK_VERSION) && (D3D_SDK_VERSION != 32)

#define SAFE_RELEASE(x) if(x) x->Release(); x = NULL;

CDMSoundObject::CDMSoundObject()
{
	m_soundLoader = NULL;
	m_soundPerformance = NULL;
	m_audioSound = NULL;
	m_audioPath = NULL;

	m_audioBuffer = NULL;
	m_audioListener = NULL;
}

/*
 * ���ʵ�ʵ��������󴴽� �������ļ��м��ص��������(��ʼ��COM ������Ĭ�ϵ���������)
 * @brief: Ҫ���ص������ļ���
	��Ҫ�ظ����ŵĴ���(0, ����һ��; -1��������ѭ������)
*/
bool CDMSoundObject::Initialize(char * filename, int numRepeats)
{
	if (filename == NULL) return false;

	char pathString[MAX_PATH];
	WCHAR path[MAX_PATH];

	// COM �����������ּ�����. Here we create the music loader object.
	/*
	* ���������ġ�δ��ʼ���ġ����͸��������������͵���
	* ������óɹ������� S_OK���ҷ��͸� *ppv ������ָ���ַ��Ϊ NULL
	* STDAPI CoCreateInstance(
	REFCLSID rclsid,
	LPUNKNOWN pUnkOuter,
	DWORD dwClsContext,
	REFIID riid,
	LPVOID *ppv
	);
	* Ҫ�����������͵� CLSID
	* ���� IUNKNOWN �ӿڵ�ָ��
	* ����Ҫ���е��´�������Ĵ���������������
	* ���ںͶ���ͨ�ŵĽӿڱ�ʶ��������
	* �ú�����Ҫ������ָ������ĵ�ַ
	*
	* IID_IDirectMusicLoader8 - �ӿ����ڴ��ļ��в��ҡ��о١�����ͼ��ض���.
	*/
	if (FAILED(CoCreateInstance(CLSID_DirectMusicLoader, NULL, CLSCTX_INPROC, IID_IDirectMusicLoader8, (void**)&m_soundLoader)))
		return false;

	// Here we create the performance object.
	// IID_IDirectMusicPerformance8 - �ӿ������ֻطŵ��ܹܣ��ڲ��š�ֹͣ����Ƭ�Σ�������ͨ��ӳ�䵽�˿ڣ���Ӻ�ɾ���˿ڣ������¼����������ֲ���ʱ��ʹ�õ��ýӿ�
	if (FAILED(CoCreateInstance(CLSID_DirectMusicPerformance, NULL, CLSCTX_INPROC, IID_IDirectMusicPerformance8, (void**)&m_soundPerformance)))
		return false;

	// Get directory of .exe then convert it to unicode string.(�ɶ��ֽ��ַ���ת����˫�ֽ��ַ���)
	::GetCurrentDirectory(MAX_PATH, pathString);
	::MultiByteToWideChar(CP_ACP, 0, pathString, -1, path, MAX_PATH);

	// Set search directory to where the sound can be found.
	/*
	* ��������ͼ���ļ�������������ʱ DirectMusic ��ʹ�õ�����·��
	* @param: �� GUID(�����е� DirectMusic ���ͣ��ò�������Ϊ GUID_DirectMusicAllTypes)
	Ҫ���õ�·��
	������·��ǰ֪ͨ���������������Ϣ�ı�ʶ��(������ǰ��Ŀ¼�а�������ͬ���ļ���)
	HRESULT SetSearchDirectory(REFGUID rguidClass, WCHAR* pwszPath, BOOL fClear);
	*/
	m_soundLoader->SetSearchDirectory(GUID_DirectMusicAllTypes, path, FALSE);

	// Initialize the audio.
	/*
	* ��ʼ�����ܣ���������ѡ�������Ĭ�ϵ���Ƶ·��.
	* �ڲ�����Ƶ֮ǰ������ɸù���(�������Ŷ������ڲ���������Ƶ)
	*
	HRESULT InitAudio(
	IDirectMusic** ppDirectMusic,
	IDirectSound** ppDirectSound,
	HWND hWnd,
	DWORD dwDefaultPathType,
	DWORD dwPChannelCount,
	DWORD dwFlags,
	DMUS_AUDIOPARAMS *pParams
	);
	* IDirectMusic ָ��
	* IDirectSound ָ��
	* ���ھ��
	* Ĭ����Ƶ·������(DMSU_APATH_DYNAMIC_3D��DMUS_APATH_DYNAMIC_MONO��DMUS_APATH_DYNAMIC_STEREO��DMUS_APATH_SHARED_STEREOPLUSREVERB)
	* Ҫʹ�õĲ���ͨ����Ŀ
	* ָ����ʼ����Ƶ��ʽ�ı�ʶ��
	* ��Ƶ����ָ��
	*
	* IDirectMusic ָ���IDirectSound ָ���ΪNULL����, ��ʹ�����������󲢽���ַ���ظ�ָ��, Ҳ���Ա�����NULL����ʹ�����������󣬲����ڲ�ʹ�ö��󣬻�����Ƿ��������ͨ���������Ч����
	*/
	if (FAILED(m_soundPerformance->InitAudio(NULL, NULL, NULL, DMUS_APATH_DYNAMIC_STEREO, 64, DMUS_AUDIOF_ALL, NULL)))
		return false;

	// Here you create the 3D audio path.
	/*
	* ������Ƶ·������(������ʾ�Ӳ��ŵ� DirectSound ������������еĽ׶ζ���)
	* ��Ƶ·���������������Ӳ��ŵ����ջ�����Ĳ�ͬ�׶Ρ�����Ҫ�����ڳ����в�������Ƭ��.
	*	HRESULT CreateStandardAudioPath(
	DWORD dwType,
	DWORD dwPChannelCount,
	BOOL fAcivate,
	IDirectMusicAudioPath **ppNewPath
	);
	* ��Ҫ��������Ƶ·������(����������������3D��)
	* Ҫʹ�õĲ���ͨ����Ŀ
	* ִ���ڴ�����Ƶ·��ʱ�Ƿ�Ҫ�������ı�ʶ��
	* Ҫ�����Ķ���ָ��
	*/
	if (m_soundPerformance->CreateStandardAudioPath(DMUS_APATH_DYNAMIC_3D, 64, TRUE, &m_audioPath) != S_OK)
		return false;

	/*
	* ��ȡ��Ƶ·���е��������󣬽�������Ƶ����[����Ϊ IDirectSound3DBuffer] (������Ƶ·����Ϊ�����ṩ�Ľӿ�)
	*	HRESULT GetObjectInPath(
	DWORD dwPChannel,
	DWORD dwStage,
	DWORD dwBuffer,
	REFGUID guidObject,
	DWORD dwIndex,
	REFGUID iidInterface,
	void** ppObject
	);
	* Ҫ�����Ĳ���ͨ��
	* ·���еĽ׶�
	* DMO פ���Ļ�������(��� dwStage ����Ϊ DMUS_PATH_BUFFER �� DMUS_PATH_MIXIN_BUFFER_DMO)
	* ��������ʶ��
	* ƥ���������Ķ�������(�����Ҫʹ�õ�һ�����ֵ��������󣬾ͽ���ֵ��Ϊ0)
	* �����ӿڵı�ʶ��
	* Ҫ�����Ķ���ָ��
	*/
	if (FAILED(m_audioPath->GetObjectInPath(0, DMUS_PATH_BUFFER, 0, GUID_NULL, 0, IID_IDirectSound3DBuffer, (void**)&m_audioBuffer)))
		return false;

	// Retrieve the 3D buffer parameters.
	/*
	* @param: Ҫ����������������(ͨ����������)�� LPD3DBUFFER ����
	HRESULT GetAllParameters(LPDS3DBUFFER pDs3dBuffer);
	*/
	m_bufferParams.dwSize = sizeof(DS3DBUFFER);
	m_audioBuffer->GetAllParameters(&m_bufferParams);

	// Set the 3D buffer parameters.
	m_bufferParams.dwMode = DS3DMODE_HEADRELATIVE;
	/*
	* @param: Ҫ���õľ������Ե� LPD3DBUFFER ����
	Ҫ�������Ե�ʱ���ʶ��(������ֶ��ύ����ı䶯������Ϊ DS3D_DEFREED, ����������ñ仯��Ч������Ϊ DS3D_IMMEDIATE)
	HRESULT SetAllParameters(LPDS3DBUFFER pDs3dBuffer, DWORD dwApply);
	*/
	m_audioBuffer->SetAllParameters(&m_bufferParams, DS3D_IMMEDIATE);

	// Set up the 3D listener.
	if (FAILED(m_audioPath->GetObjectInPath(0, DMUS_PATH_PRIMARY_BUFFER, 0, GUID_NULL, 0, IID_IDirectSound3DListener, (void**)&m_audioListener)))
		return false;

	// Retrieve the 3D listener parameters.
	m_listenerParams.dwSize = sizeof(DS3DLISTENER);
	m_audioListener->GetAllParameters(&m_listenerParams);

	// Set the 3D listener parameters.
	m_listenerParams.vPosition.x = 0.0f;
	m_listenerParams.vPosition.y = 0.0f;
	m_listenerParams.vPosition.z = 0.0f;
	m_audioListener->SetAllParameters(&m_listenerParams, DS3D_IMMEDIATE);

	// You must convert the filename to a unicode.
	::MultiByteToWideChar(CP_ACP, 0, filename, -1, path, MAX_PATH);

	// Load sound from file.
	/*
	* @brief: ���ļ��м�����������
	HRESULT LoadObjectFromFile(
	REFGUID rguidClassID,
	REFID iidInterfaceID,
	WCHAR *pwzFilePath,
	void **ppObject
	);
	* Ψһ��ʶ����
	* �ӿ�Ψһ��ʶ��
	* �ļ���
	* ����ָ��ҪΪ����������ݵ�ָ��ı�����ַ
	*/
	if (m_soundLoader->LoadObjectFromFile(CLSID_DirectMusicSegment, IID_IDirectMusicSegment8, path, (void**)&m_audioSound) != S_OK)
		return false;

	// Set repeats if desired.
	/*
	* HRESULT SetRepeats(DWORD dwRepeats);
	* @param: 0/1, ����ֻ����һ�Ρ�DMUS_SEG_REPEAT_INFINITE, �����Ƶ�ѭ����������. ��������ֵΪ���Ŵ���
	*/
	if (numRepeats < 0)
		m_audioSound->SetRepeats(DMUS_SEG_REPEAT_INFINITE);
	else
		m_audioSound->SetRepeats(numRepeats);

	// Download the band to the synthesizer.
	/*
	* HRESULT Download(IUnknown* pAudioPath);
	* ����Ƶ�������ص����Ż���Ƶ·���ϣ�������Я������·������ʹ�����ƵƬ�϶���(����Ϊ IDirectMusicSegment8 ����Ƶ����)
	* @param: �������ص���Ƶ·��
	*/
	m_audioSound->Download(m_audioPath);

	// Setup for 3D sound (default).
	if (!SetupSoundParameters(0.0f, 0.1f, 1.0f, 100.0f))
		return false;

	return true;
}

/*
 * @brief: ���������Ķ�����ЧӦ�͵���ЧӦ(����3D����)
		����3D��������������������������������õ����������������
 * @param: ���������ӡ��������ӡ�����֮��������С����
 */
bool CDMSoundObject::SetupSoundParameters(float dopplerFactor, float rolloffFactor, float minDist, float maxDist)
{
	// Set listener.
	m_listenerParams.flDopplerFactor = dopplerFactor;
	m_listenerParams.flRolloffFactor = rolloffFactor;
	m_audioListener->SetAllParameters(&m_listenerParams, DS3D_IMMEDIATE);

	// Set buffer.
	m_bufferParams.flMinDistance = minDist;
	m_bufferParams.flMaxDistance = maxDist;
	m_audioBuffer->SetAllParameters(&m_bufferParams, DS3D_IMMEDIATE);

	return true;
}

// ������������
void CDMSoundObject::Play()
{
	// If it is already playing, return.
	/*
	 * HRESULT IsPlaying(IDirectMusicSegment* pSegment, IDirectMusicSegmentState* pSegState);
	 * �Ƿ��Ѿ��������ڲ��ŵ������ļ�(���ڲ�������)
	 * @brief: IDirectMusicPerformance8 ����������Ҫ����Ƭ�����(NULL, ֻ����һ������)
	 */
	if (m_soundPerformance->IsPlaying(m_audioSound, NULL) == S_OK)
		return;

	/*
	 * ��������
	 * @brief: Ҫ���ŵ� IDirectMusicSegment ����
		֪ͨ������������ʼ���ŵ�ʱ��(����ʱ��)(��ֵΪ0�����̿�ʼ����)
		���ղ�������ʵ��Ƭ������ָ��ı�����ַ

		HRESULT PlaySegment(
			IDirectMusicSegment* pSegment,
			DWORD dwFlags,
			__int64 i64StartTime,
			IDirectMusicSegmentState** ppSegmentState
		);
	 * PlaySegmentEx
	 */
	m_soundPerformance->PlaySegmentEx(m_audioSound, NULL, NULL, DMUS_SEGF_DEFAULT, 0, NULL, NULL, m_audioPath);
}

// ����������3D�ռ��е�ԭ��
void CDMSoundObject::UpdateSoundPosition(float x, float y, float z)
{
	// Set sound position.
	/*
	 * HRESULT SetPosition(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
	 * @param: ��������λ�������ᣬָ��Ч������ʱ�̵ı�ʶ��(DS3D_DEFERRED/DS3D_IMMEDIATE)
	 */
	if (m_audioBuffer != NULL)
		m_audioBuffer->SetPosition(x, y, z, DS3D_IMMEDIATE);
}

// ֹͣ��ǰ���ڲ��ŵ�������������
void CDMSoundObject::Stop()
{
	// Only stop is it is playing.
	if (m_soundPerformance->IsPlaying(m_audioSound, NULL) == S_OK)
		/*
		 * ֹͣ��������
		 * HRESULT Stop(
				IDirectMusicSegment* pSegment,
				IDirectMusicSegmentState* pSegmentState,
				MUSIC_TIME mtTime,
				DWORD dwFlags
			);

			HRESULT StopEx(
				IUnknown *pObjectToStop,
				__int64 i64StopTime,
				DWORD dwFlags
			);
		 */
		m_soundPerformance->StopEx(m_audioSound, 0, 0);
}

void CDMSoundObject::Shutdown()
{
	// Release all resources.
	if (m_soundPerformance)
	{
		Stop();
		m_soundPerformance->CloseDown();
	}

	SAFE_RELEASE(m_soundLoader);
	SAFE_RELEASE(m_soundPerformance);
	SAFE_RELEASE(m_audioSound);
	SAFE_RELEASE(m_audioPath);

	m_soundPerformance = NULL;
}

//////////////////////////////////////////////////////////////////////////

CDirectMusicSystem::CDirectMusicSystem()
{
	m_comInit = false;
	m_totalSounds = 0;
	m_soundList = nullptr;
}

int CDirectMusicSystem::IncreaseSounds()
{
	// This function increases the m_sound array.
	if (!m_soundList)
	{
		m_soundList = new CDMSoundObject[1];
		if (!m_soundList) return STRANDED_FAIL;
	}
	else
	{
		CDMSoundObject *temp;
		temp = new CDMSoundObject[m_totalSounds + 1];
		if (!temp) return STRANDED_FAIL;

		memcpy(temp, m_soundList, sizeof(CDMSoundObject)*m_totalSounds);

		delete[] m_soundList;
		m_soundList = temp;
	}

	return STRANDED_OK;
}

bool CDirectMusicSystem::AddSound(char *soundfile, int numRepeats, int *id)
{
	if (!m_comInit)
	{
		// Initialize COM for DirectMusic.
		if (FAILED(::CoInitialize(NULL))) return false;
		m_comInit = true;
	}

	if (!IncreaseSounds())
		return false;

	if (!m_soundList[m_totalSounds].Initialize(soundfile, numRepeats))
		return false;

	if (id)
		*id = m_totalSounds;

	m_totalSounds++;

	return true;
}

bool CDirectMusicSystem::SetupSoundParameters(int id, float dopplerFactor, float rolloffFactor, float minDist, float maxDist)
{
	if (id >= m_totalSounds)
		return false;

	return m_soundList[id]->SetupSoundParameters(dopplerFactor, rolloffFactor, minDist, maxDist);
}

void CDirectMusicSystem::Play(int id)
{
	if (id >= m_totalSounds)
		return;

	m_soundList[id]->Play();
}

void CDirectMusicSystem::UpdateSoundPosition(int id, float x, float y, float z)
{
	if (id >= m_totalSounds)
		return;

	m_soundList[id]->UpdateSoundPosition(x, y, z);
}

void CDirectMusicSystem::Stop(int id)
{
	if (id >= m_totalSounds)
		return;

	m_soundList[id]->Stop();
}

void CDirectMusicSystem::Shutdown()
{
	for (int i = 0; i < m_totalSounds; i++)
	{
		m_soundList[i].Stop();
		m_soundList[i].Shutdown();
	}

	if (m_soundList)
	{
		delete[] m_soundList;
		m_soundList = nullptr;
	}

	m_totalSounds = 0;

	if (m_comInit)
		::CoUninitialize();
}


//#ifdef
//#ifndef
//#elif
#else
//
#endif

bool CreateDMSound(CSoundSystemInterface **pObj)
{
	if (!*pObj)
		*pObj = new CDirectMusicSystem();
	else
		return false;

	return true;
}

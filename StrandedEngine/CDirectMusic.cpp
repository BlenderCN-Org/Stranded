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
 * 完成实际的声音对象创建 将声音文件夹加载到类对象中(初始化COM 和设置默认的声音参数)
 * @brief: 要加载的声音文件名
	想要重复播放的次数(0, 播放一次; -1，无限制循环播放)
*/
bool CDMSoundObject::Initialize(char * filename, int numRepeats)
{
	if (filename == NULL) return false;

	char pathString[MAX_PATH];
	WCHAR path[MAX_PATH];

	// COM 函数创建音乐加载器. Here we create the music loader object.
	/*
	* 创建单个的、未初始化的、发送给函数的任意类型的类
	* 如果调用成功，返回 S_OK，且发送给 *ppv 参数的指针地址不为 NULL
	* STDAPI CoCreateInstance(
	REFCLSID rclsid,
	LPUNKNOWN pUnkOuter,
	DWORD dwClsContext,
	REFIID riid,
	LPVOID *ppv
	);
	* 要创建的类类型的 CLSID
	* 对象 IUNKNOWN 接口的指针
	* 管理将要运行的新创建对象的代码所处的上下文
	* 用于和对象通信的接口标识符的引用
	* 该函数所要创建的指针变量的地址
	*
	* IID_IDirectMusicLoader8 - 接口用于从文件中查找、列举、缓存和加载对象.
	*/
	if (FAILED(CoCreateInstance(CLSID_DirectMusicLoader, NULL, CLSCTX_INPROC, IID_IDirectMusicLoader8, (void**)&m_soundLoader)))
		return false;

	// Here we create the performance object.
	// IID_IDirectMusicPerformance8 - 接口是音乐回放的总管，在播放、停止音乐片段，将播放通道映射到端口，添加和删除端口，处理事件，处理音乐参数时将使用到该接口
	if (FAILED(CoCreateInstance(CLSID_DirectMusicPerformance, NULL, CLSCTX_INPROC, IID_IDirectMusicPerformance8, (void**)&m_soundPerformance)))
		return false;

	// Get directory of .exe then convert it to unicode string.(由多字节字符串转换成双字节字符串)
	::GetCurrentDirectory(MAX_PATH, pathString);
	::MultiByteToWideChar(CP_ACP, 0, pathString, -1, path, MAX_PATH);

	// Set search directory to where the sound can be found.
	/*
	* 设置在试图从文件加载声音对象时 DirectMusic 将使用的搜索路径
	* @param: 类 GUID(对所有的 DirectMusic 类型，该参数可设为 GUID_DirectMusicAllTypes)
	要设置的路径
	在设置路径前通知加载器清除对象信息的标识符(避免先前的目录中包含了相同的文件名)
	HRESULT SetSearchDirectory(REFGUID rguidClass, WCHAR* pwszPath, BOOL fClear);
	*/
	m_soundLoader->SetSearchDirectory(GUID_DirectMusicAllTypes, path, FALSE);

	// Initialize the audio.
	/*
	* 初始化性能，并可以有选择地设置默认的音频路径.
	* 在播放音频之前必须完成该工作(声音播放对象用于播放声音音频)
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
	* IDirectMusic 指针
	* IDirectSound 指针
	* 窗口句柄
	* 默认音频路径类型(DMSU_APATH_DYNAMIC_3D、DMUS_APATH_DYNAMIC_MONO、DMUS_APATH_DYNAMIC_STEREO、DMUS_APATH_SHARED_STEREOPLUSREVERB)
	* 要使用的播放通道数目
	* 指定初始化音频方式的标识符
	* 音频参数指针
	*
	* IDirectMusic 指针和IDirectSound 指针可为NULL变量, 会使函数创建对象并将地址返回给指针, 也可以被传递NULL，会使函数创建对象，并在内部使用对象，会可以是分配给声音通道对象的有效对象
	*/
	if (FAILED(m_soundPerformance->InitAudio(NULL, NULL, NULL, DMUS_APATH_DYNAMIC_STEREO, 64, DMUS_AUDIOF_ALL, NULL)))
		return false;

	// Here you create the 3D audio path.
	/*
	* 创建音频路径对象(创建表示从播放到 DirectSound 缓存的数据流中的阶段对象)
	* 音频路径代表了数据流从播放到最终混合器的不同阶段。它主要用于在程序中播放声音片断.
	*	HRESULT CreateStandardAudioPath(
	DWORD dwType,
	DWORD dwPChannelCount,
	BOOL fAcivate,
	IDirectMusicAudioPath **ppNewPath
	);
	* 正要创建的音频路径类型(立体声、单声道、3D等)
	* 要使用的播放通道数目
	* 执行在创建音频路径时是否要激活它的标识符
	* 要创建的对象指针
	*/
	if (m_soundPerformance->CreateStandardAudioPath(DMUS_APATH_DYNAMIC_3D, 64, TRUE, &m_audioPath) != S_OK)
		return false;

	/*
	* 获取音频路径中的声音对象，将创建音频缓存[类型为 IDirectSound3DBuffer] (接收音频路径中为对象提供的接口)
	*	HRESULT GetObjectInPath(
	DWORD dwPChannel,
	DWORD dwStage,
	DWORD dwBuffer,
	REFGUID guidObject,
	DWORD dwIndex,
	REFGUID iidInterface,
	void** ppObject
	);
	* 要搜索的播放通道
	* 路径中的阶段
	* DMO 驻留的缓存索引(如果 dwStage 参数为 DMUS_PATH_BUFFER 活 DMUS_PATH_MIXIN_BUFFER_DMO)
	* 对象的类标识符
	* 匹配对象链表的对象索引(如果想要使用第一个发现的声音对象，就将该值设为0)
	* 期望接口的标识符
	* 要创建的对象指针
	*/
	if (FAILED(m_audioPath->GetObjectInPath(0, DMUS_PATH_BUFFER, 0, GUID_NULL, 0, IID_IDirectSound3DBuffer, (void**)&m_audioBuffer)))
		return false;

	// Retrieve the 3D buffer parameters.
	/*
	* @param: 要设置声音缓存属性(通过函数调用)的 LPD3DBUFFER 对象
	HRESULT GetAllParameters(LPDS3DBUFFER pDs3dBuffer);
	*/
	m_bufferParams.dwSize = sizeof(DS3DBUFFER);
	m_audioBuffer->GetAllParameters(&m_bufferParams);

	// Set the 3D buffer parameters.
	m_bufferParams.dwMode = DS3DMODE_HEADRELATIVE;
	/*
	* @param: 要设置的具有属性的 LPD3DBUFFER 对象、
	要设置属性的时间标识符(如果想手动提交程序的变动，可设为 DS3D_DEFREED, 如果想立刻让变化生效，可设为 DS3D_IMMEDIATE)
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
	* @brief: 从文件中加载声音数据
	HRESULT LoadObjectFromFile(
	REFGUID rguidClassID,
	REFID iidInterfaceID,
	WCHAR *pwzFilePath,
	void **ppObject
	);
	* 唯一标识符类
	* 接口唯一标识符
	* 文件名
	* 接收指向要为对象加载数据的指针的变量地址
	*/
	if (m_soundLoader->LoadObjectFromFile(CLSID_DirectMusicSegment, IID_IDirectMusicSegment8, path, (void**)&m_audioSound) != S_OK)
		return false;

	// Set repeats if desired.
	/*
	* HRESULT SetRepeats(DWORD dwRepeats);
	* @param: 0/1, 声音只播放一次。DMUS_SEG_REPEAT_INFINITE, 无限制地循环播放声音. 其他任意值为播放次数
	*/
	if (numRepeats < 0)
		m_audioSound->SetRepeats(DMUS_SEG_REPEAT_INFINITE);
	else
		m_audioSound->SetRepeats(numRepeats);

	// Download the band to the synthesizer.
	/*
	* HRESULT Download(IUnknown* pAudioPath);
	* 将音频数据下载到播放或音频路径上，可用于携带声音路径对象和创建音频片断对象(类型为 IDirectMusicSegment8 的音频对象)
	* @param: 正在下载的音频路径
	*/
	m_audioSound->Download(m_audioPath);

	// Setup for 3D sound (default).
	if (!SetupSoundParameters(0.0f, 0.1f, 1.0f, 100.0f))
		return false;

	return true;
}

/*
 * @brief: 设置声音的多普勒效应和跌落效应(负责3D声音)
		设置3D声音和侦听器缓存参数，并将他们运用到声音缓存和侦听器
 * @param: 多普勒因子、跌落因子、它们之间的最大、最小距离
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

// 播放声音对象
void CDMSoundObject::Play()
{
	// If it is already playing, return.
	/*
	 * HRESULT IsPlaying(IDirectMusicSegment* pSegment, IDirectMusicSegmentState* pSegState);
	 * 是否已经包含正在播放的声音文件(正在播放声音)
	 * @brief: IDirectMusicPerformance8 声音对象、想要检查的片断语句(NULL, 只检查第一个参数)
	 */
	if (m_soundPerformance->IsPlaying(m_audioSound, NULL) == S_OK)
		return;

	/*
	 * 播放声音
	 * @brief: 要播放的 IDirectMusicSegment 对象
		通知函数该声音开始播放的时间(音乐时间)(该值为0，立刻开始播放)
		接收播放声音实例片断语句的指针的变量地址

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

// 更新声音在3D空间中的原点
void CDMSoundObject::UpdateSoundPosition(float x, float y, float z)
{
	// Set sound position.
	/*
	 * HRESULT SetPosition(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
	 * @param: 声音出处位置坐标轴，指明效果发生时刻的标识符(DS3D_DEFERRED/DS3D_IMMEDIATE)
	 */
	if (m_audioBuffer != NULL)
		m_audioBuffer->SetPosition(x, y, z, DS3D_IMMEDIATE);
}

// 停止当前正在播放的所有声音对象
void CDMSoundObject::Stop()
{
	// Only stop is it is playing.
	if (m_soundPerformance->IsPlaying(m_audioSound, NULL) == S_OK)
		/*
		 * 停止播放声音
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

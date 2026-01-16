#include "SoundManager.h"

#pragma comment(lib,"mfplat.lib")
#pragma comment(lib,"Mfreadwrite.lib")
#pragma comment(lib,"mfuuid.lib")

void SoundManager::Initialize() {
	//XAudio2の初期化
	IXAudio2MasteringVoice* masterVoice = nullptr;
	HRESULT result;
	// XAudioエンジンのインスタンスを生成
	result = XAudio2Create(xAudio2.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));
	// マスターボイスを生成
	result = xAudio2->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(result));
	// Windows Media Foundationの初期化(ローカルファイル版)
	result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	assert(SUCCEEDED(result));
}

// 音声再生
void SoundManager::SoundPlayWave(const SoundData& soundData) {
	HRESULT result;

	// 波形フォーマットを元にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer.data();
	buf.AudioBytes = static_cast<UINT32>(soundData.pBuffer.size());
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// 波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
}

// 音声データの読み込み
SoundData SoundManager::SoundLoadFile(const std::string& filename) {
	std::string fullpath = ("Resource/Sounds/") + filename;
	
	// フルパスをワイド文字列に変換
	std::wstring filePathW = StringUtility::ConvertString(fullpath);
	HRESULT result;

	// SourceReader作成
	Microsoft::WRL::ComPtr<IMFSourceReader> pReader;
	result = MFCreateSourceReaderFromURL(filePathW.c_str(), nullptr, pReader.GetAddressOf());
	assert(SUCCEEDED(result));

	// PCM形式にフォーマット指定する
	Microsoft::WRL::ComPtr<IMFMediaType> pPCMType;
	MFCreateMediaType(&pPCMType);
	pPCMType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	pPCMType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	result = pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr,pPCMType.Get());
	assert(SUCCEEDED(result));

	// 実際にセットされたメディアタイプを取得する
	Microsoft::WRL::ComPtr<IMFMediaType> pOutType;
	pReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pOutType);

	// Waveフォーマットを取得する
	WAVEFORMATEX* waveFormat = nullptr;
	MFCreateWaveFormatExFromMFMediaType(pOutType.Get(), &waveFormat, nullptr);

	// コンテナに格納する音声データ
	SoundData soundData = {};
	soundData.wfex = *waveFormat;

	// 生成したWaveフォーマットを解放
	CoTaskMemFree(waveFormat);

	// PCMデータのバッファを構築
	while (true) {
		Microsoft::WRL::ComPtr<IMFSample> pSample;
		DWORD streamIndex = 0, flags = 0;
		LONGLONG llTimeStamp = 0;
		// サンプルを読み込む
		result = pReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &streamIndex, &flags, &llTimeStamp, &pSample);
		// ストリームの末尾に達したら抜ける
		if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break;

		if (pSample) {
			Microsoft::WRL::ComPtr<IMFMediaBuffer> pBuffer;
			// サンプルに含まれるっサウンドデータのバッファを一繋ぎにして取得
			pSample->ConvertToContiguousBuffer(&pBuffer);

			BYTE* pData = nullptr; // データ読み取り用ポインタ
			DWORD maxLength = 0, currentLength = 0;
			// バッファ読み込み用にロック
			pBuffer->Lock(&pData, &maxLength, &currentLength);
			// バッファの末尾にデータを追加
			soundData.pBuffer.insert(soundData.pBuffer.end(), pData, pData + currentLength);
			pBuffer->Unlock();
		}
	}


	return soundData;

}

// 音声データ解放
void SoundManager::SoundUnload(SoundData* soundData) {
	soundData->pBuffer.clear();
	soundData->wfex = {};
}

void SoundManager::Finalize(SoundData* soundData) {
	// 後始末
	HRESULT result;
	// Windows Media Foundationの終了
	result = MFShutdown();
	assert(SUCCEEDED(result));

	// 音声データ解放
	SoundUnload(soundData);
	// 音声データ解放
	xAudio2.Reset();
}

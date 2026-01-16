#include "SoundManager.h"

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
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// 波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
}
// 音声データの読み込み
SoundData SoundManager::SoundLoadWave(const char* filename) {
	std::ifstream file(filename, std::ios_base::binary);
	assert(file.is_open());

	RiffHeader riff{};
	file.read(reinterpret_cast<char*>(&riff), sizeof(riff));
	assert(strncmp(riff.chunk.id, "RIFF", 4) == 0);
	assert(strncmp(riff.type, "WAVE", 4) == 0);

	ChunkHeader fmtHeader{};
	file.read(reinterpret_cast<char*>(&fmtHeader), sizeof(fmtHeader));
	assert(strncmp(fmtHeader.id, "fmt ", 4) == 0);

	std::vector<char> fmtData(fmtHeader.size);
	file.read(fmtData.data(), fmtHeader.size);

	WAVEFORMATEX* wfex = reinterpret_cast<WAVEFORMATEX*>(fmtData.data());

	SoundData soundData{};
	size_t copySize = fmtHeader.size < sizeof(WAVEFORMATEX) ? fmtHeader.size : sizeof(WAVEFORMATEX);
	memcpy(&soundData.wfex, wfex, copySize);

	if (fmtHeader.size > sizeof(WAVEFORMATEX)) {
		soundData.wfex.cbSize = *reinterpret_cast<WORD*>(fmtData.data() + sizeof(WAVEFORMATEX));
	} else {
		soundData.wfex.cbSize = 0;
	}

	ChunkHeader dataHeader{};
	while (true) {
		file.read(reinterpret_cast<char*>(&dataHeader), sizeof(dataHeader));
		if (strncmp(dataHeader.id, "data", 4) == 0) {
			break;
		}
		file.seekg(dataHeader.size, std::ios_base::cur);
	}

	assert(dataHeader.size < 100 * 1024 * 1024); // 100MB制限など適宜

	char* pBuffer = new char[dataHeader.size];
	file.read(pBuffer, dataHeader.size);
	assert(file.gcount() == dataHeader.size);

	file.close();

	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = dataHeader.size;

	return soundData;
}

// 音声データ解放
void SoundManager::SoundUnload(SoundData* soundData) {
	// バッファのメモリを解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

void SoundManager::Finalize(SoundData* soundData) {
	// 音声データ解放
	SoundUnload(soundData);
	// 音声データ解放
	xAudio2.Reset();
}

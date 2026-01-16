#pragma once
#include <xaudio2.h>
#include <wrl.h>
#include <cassert>
#include <fstream>
#include <vector>

// 音声データ
struct SoundData {
	// 波形フォーマット
	WAVEFORMATEX wfex;
	// バッファの先頭アドレス
	BYTE* pBuffer;
	// バッファのサイズ
	unsigned int bufferSize;
};
// チャンクヘッダ
struct ChunkHeader {
	char id[4]; // チャンク毎のID
	int32_t size; // チャンクサイズ
};
// RIFFヘッダチャンク
struct RiffHeader {
	ChunkHeader chunk; // RIFF
	char type[4]; // WAVE
};
// フォーマットチャンク
struct FormatChunk {
	ChunkHeader chunk; // "fmt "チャンクヘッダー
	WAVEFORMATEX  fmt; // フォーマット本体（最大40バイト程度）
};

class SoundManager {
public:
	// 初期化
	void Initialize();
	// 音声再生
	void SoundPlayWave(const SoundData& soundData);
	// 音声データの読み込み
	SoundData SoundLoadWave(const char* filename);

	// 音声データ解放
	void SoundUnload(SoundData* soundData);

	// 終了
	void Finalize(SoundData* soundData);

private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
};


#pragma once
#include <windows.h>
#include <xaudio2.h>
#include <wrl.h>
#include <cassert>
#include <fstream>
#include <vector>
#include <mfapi.h>
#include <mfidl.h> 
#include <mfreadwrite.h>

#include "StringUtility.h"

// 音声データ
struct SoundData {
	// 波形フォーマット
	WAVEFORMATEX wfex;
	// バッファの先頭アドレス
	std::vector<BYTE> pBuffer;
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
	SoundData SoundLoadFile(const std::string& filename);

	// シングルトンインスタンスの取得
	static SoundManager* GetInstance();

	// 終了
	void Finalize(SoundData* soundData);

private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;

	// シングルトンインスタンス
	static SoundManager* instance;

	// 音声データ解放
	void SoundUnload(SoundData* soundData);
};


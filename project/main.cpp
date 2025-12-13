#define _USE_MATH_DEFINES

#include <Windows.h>
#include <cstdint>
#include <string>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <D3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dbghelp.h>
#include <strsafe.h>
#include <dxgidebug.h>
#include <dxcapi.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <sstream>
#include <wrl.h>
#include <xaudio2.h>
#include <dinput.h>
#include <cmath>

#include "DebugCamera.h"
#include "Calc.h"
#include "Input.h"
#include "WindowAPI.h"
#include "DirectXCommon.h"
#include "SpriteCommon.h"
#include "Sprite.h"

#include "externals/imgui\imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma comment(lib,"D3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"Dbghelp.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")
#pragma comment(lib,"xaudio2.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")


//文字列を格納する
std::string str0{ "STRING" };

//ログのディレクトリを用意
namespace fs = std::filesystem;

// 3x3行列
struct Matrix3x3 {
	float m[3][3] = { 0 };
};

// transformの初期化
Transform transform
{
	{ 1.0f, 1.0f, 1.0f }, // scale
	{ 0.0f, 0.0f, 0.0f }, // rotate
	{ 0.0f, 0.0f, 0.0f }  // translate
};

// transformSpriteの初期化
Transform tranaformSprite
{
	{1.0f,1.0f,1.0f},
	{0.0f,0.0f,0.0f},
	{0.0f,0.0f,0.0f}
};


// cameraTransformの初期化
Transform cameraTransform
{
	{ 1.0f, 1.0f, 1.0f }, // scale
	{ 0.0f, 0.0f, 0.0f }, // rotate
	{ 0.0f, 0.0f, -5.0f } // translate
};

// 頂点データ
struct VertexData {
	Vector4 position; // 頂点座標
	Vector2 texcoord; // テクスチャ座標
	Vector3 normal; // 正規化座標
};

struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

struct DirectionalLight {
	Vector4 color; // ライトの色
	Vector3 direction; // ライトの向き
	float intensity; // 輝度
};

struct MaterialData {
	std::string textureFilePath;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

// チャンクヘッダ
struct ChunkHeader {
	char id[4]; // チャンク毎のID
	int32_t size; // チャンクサイズ
};

// フォーマットチャンク
struct FormatChunk {
	ChunkHeader chunk; // "fmt "チャンクヘッダー
	WAVEFORMATEX  fmt; // フォーマット本体（最大40バイト程度）
};

// RIFFヘッダチャンク
struct RiffHeader {
	ChunkHeader chunk; // RIFF
	char type[4]; // WAVE
};

// 音声データ
struct SoundData {
	// 波形フォーマット
	WAVEFORMATEX wfex;
	// バッファの先頭アドレス
	BYTE* pBuffer;
	// バッファのサイズ
	unsigned int bufferSize;
};


Transform uvTransformSprite{
	{1.0f,1.0f,1.0f},
	{0.0f,0.0f,0.0f},
	{0.0f,0.0f,0.0f},
};

// SRV切り替え
bool useMonsterBall = true;

static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Duumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	// processId(このexeのID)とクラッシュ（例外）の発生したthreadIdを取得
	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();
	// 設定情報を入力
	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
	minidumpInformation.ThreadId = threadId;
	minidumpInformation.ExceptionPointers = exception;
	minidumpInformation.ClientPointers = TRUE;
	//Dumpを出力。MiniDumpNormalは最低限の情報を出力するプラグ
	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInformation, nullptr, nullptr);

	return EXCEPTION_EXECUTE_HANDLER;
}

void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}


void Log(std::ostream& os, const std::string& message) {
	os << message << std::endl;
	OutputDebugStringA(message.c_str());
}

// ウィンドウプロシージャ
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) {
		return true;
	}

	// メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		// ウィンドウが破壊された
	case WM_DESTROY:
	// OSに対して、アプリの終了を伝える
	PostQuitMessage(0);
	return 0;
	}
	// 標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wParam, lParam);

}



Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device>& device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}




// 正規化関数
void Normalize(float& x, float& y, float& z) {
	float len = std::sqrt(x * x + y * y + z * z);
	if (len > 0.00001f) {
		x /= len;
		y /= len;
		z /= len;
	}
}

// 球を描画する関数
void DrawSphere(VertexData vertexData[]) {
	const uint32_t kSubdivision = 20;
	const float kLonEvery = 2.0f * float(M_PI) / float(kSubdivision);
	const float kLatEvery = float(M_PI) / float(kSubdivision);

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = float(M_PI) / 2.0f - kLatEvery * latIndex;
		float latNext = float(M_PI) / 2.0f - kLatEvery * (latIndex + 1);

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			float lon = kLonEvery * lonIndex;
			float lonNext = kLonEvery * (lonIndex + 1);

			// a
			VertexData a;
			a.position = { cos(lat) * cos(lon), sin(lat), cos(lat) * sin(lon), 1.0f };
			a.texcoord = { 1.0f - (lon / (2.0f * float(M_PI))), 1.0f - ((lat + float(M_PI) / 2.0f) / float(M_PI)) };
			a.normal = { a.position.x, a.position.y, a.position.z };

			// b
			VertexData b;
			b.position = { cos(latNext) * cos(lon), sin(latNext), cos(latNext) * sin(lon), 1.0f };
			b.texcoord = { 1.0f - (lon / (2.0f * float(M_PI))), 1.0f - ((latNext + float(M_PI) / 2.0f) / float(M_PI)) };
			b.normal = { b.position.x, b.position.y, b.position.z };

			// c
			VertexData c;
			c.position = { cos(lat) * cos(lonNext), sin(lat), cos(lat) * sin(lonNext), 1.0f };
			c.texcoord = { 1.0f - (lonNext / (2.0f * float(M_PI))), 1.0f - ((lat + float(M_PI) / 2.0f) / float(M_PI)) };
			c.normal = { c.position.x, c.position.y, c.position.z };

			// d
			VertexData d;
			d.position = { cos(latNext) * cos(lonNext), sin(latNext), cos(latNext) * sin(lonNext), 1.0f };
			d.texcoord = { 1.0f - (lonNext / (2.0f * float(M_PI))), 1.0f - ((latNext + float(M_PI) / 2.0f) / float(M_PI)) };
			d.normal = { d.position.x, d.position.y, d.position.z };

			// 三角形1: a, b, c
			vertexData[start + 0] = a;
			vertexData[start + 1] = b;
			vertexData[start + 2] = c;

			// 三角形2: c, b, d
			vertexData[start + 3] = d;

		}
	}
}


// mtlファイルを読む関数
MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	MaterialData materialData; // 構築するMaterialData
	std::string line; // ファイルから読んだ１行を格納するもの
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open()); // とりあえず聞けなかったら止める

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに大路多処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}

	}

	return materialData;
}

// objファイルを読む関数
ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData; // 構築するModelData
	std::vector<Vector4> positions; //位置
	std::vector<Vector3> normals; // 法線
	std::vector<Vector2> texcoords; //　テクスチャ座標
	std::string line; // ファイルから読んだ1行を格納するもの

	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open()); // とりあえず開けなかったら止める

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭の識別子を読む

		// identifierに応じた処理
		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			position.x *= -1.0f;
			positions.push_back(position);
		} else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		} else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);
		} else if (identifier == "f") {
			VertexData triangle[3];
			// 面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/'); // 区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}
				// 要素へのIndexから、実際の要素の値を取得して、頂点を構成する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				texcoord.y = 1.0f - texcoord.y;
				triangle[faceVertex] = { position,texcoord,normal };
			}

			// 頂点を逆順で登録することで、回り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		} else if (identifier == "mtllib") {
			// materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			// 基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}

	return modelData;
}

// 音声データの読み込み
SoundData SoundLoadWave(const char* filename) {
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
void SoundUnload(SoundData* soundData) {
	// バッファのメモリを解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

// 音声再生
void SoundPlayWave(Microsoft::WRL::ComPtr<IXAudio2> xAudio2, const SoundData& soundData) {
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

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	// ポインタ
	Input* input = nullptr; // input
	WindowAPI* windowAPI = nullptr; // windowAPI
	DirectXCommon* dxCommon = nullptr; // directXCommon


	// リソースリークチェック
	struct D3DResourceLeakChecker {
		~D3DResourceLeakChecker() {
			Microsoft::WRL::ComPtr <IDXGIDebug1> debug;
			if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
				debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
				debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
				debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
			}
		}
	};


	// COMの初期化
	CoInitializeEx(0, COINIT_MULTITHREADED);

	// 誰も補足しなかった場合に(Unhandled),補足する関数を登録
	SetUnhandledExceptionFilter(ExportDump);

	// WindowAPIの初期化
	windowAPI = new WindowAPI();
	windowAPI->Initialize();

	// DirectXの初期化
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(windowAPI);


	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

	// 現在時刻を取得
	//std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	// ログファイルの名前にコンマ何秒はいらないので、削って秒にする
	//std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		//nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	// 日本時間（PCの設定時間)に変換
	//std::chrono::zoned_time localTime{ std::chrono::current_zone(),nowSeconds };
	// formatを使って年月日_時分秒の文字列に変換
	//std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
	// 時刻を使てファイル名を決定
	//std::string logFilePath = std::string("logs/") + dateString + ".log";
	// ファイルを使って書き込み準備
	//std::ofstream logStream(logFilePath);

	// 出力ウィンドウへの文字出力
	//OutputDebugStringA("Hello,DirectX!\n");

	// ログ
	//bool logs = fs::create_directory("logs");
	//Log(logStream, logFilePath);

	//DXGIファクトリーの生成
	//Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	// HRESULTはWindows系のエラーコードであり、
	// 関数が成功したかどうかをSUCCEEDEDマクロで判定できる
	//hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));

	//XAudio2の初期化
	//Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	//IXAudio2MasteringVoice* masterVoice = nullptr;
	//HRESULT result;
	// XAudioエンジンのインスタンスを生成
	//result = XAudio2Create(xAudio2.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
	//assert(SUCCEEDED(result));
	// マスターボイスを生成
	//result = xAudio2->CreateMasteringVoice(&masterVoice);
	//assert(SUCCEEDED(result));

	// DirectXの初期化

	// Input初期化
	input = new Input();
	input->Initialize(windowAPI);


	

	

	//* モデル *//

	// インデックス
	Microsoft::WRL::ComPtr<ID3D12Resource> indexVertexResource = dxCommon->CreateBufferResource(sizeof(uint32_t) * 2400);

	D3D12_INDEX_BUFFER_VIEW indexBufferViewVertex{};
	// リソースの先頭のアドレスから使う
	indexBufferViewVertex.BufferLocation = indexVertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス６つ分のサイズ
	indexBufferViewVertex.SizeInBytes = sizeof(uint32_t) * 2400;
	// インデックスはuint32_tとする
	indexBufferViewVertex.Format = DXGI_FORMAT_R32_UINT;

	// インデックスリソースにデータを書き込む
	uint32_t* indexDataVertex = nullptr;
	indexVertexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexDataVertex));

	// 球のインデックスデータを設定する
	for (uint32_t i = 0; i < 2400; i += 6) {
		indexDataVertex[i + 0] = i + 0;
		indexDataVertex[i + 1] = i + 1;
		indexDataVertex[i + 2] = i + 2;
		indexDataVertex[i + 3] = i + 2;
		indexDataVertex[i + 4] = i + 1;
		indexDataVertex[i + 5] = i + 3;
	}

	// モデル読み込み
	ModelData modelData = LoadObjFile("Resource", "plane.obj");
	// 頂点バッファ用リソースを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());

	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	
	// マテリアル用のリソースを作る。今回はcolor１つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = dxCommon->CreateBufferResource(sizeof(Material));
	// マテリアルにデータを書き込む
	Material* materialData = nullptr;
	// 書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	// 色を設定する
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	// Lightingするかどうか
	materialData->enableLighting = true;
	// UVTransform行列
	materialData->uvTransform = MakeIdentity4x4();


	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = dxCommon->CreateBufferResource(sizeof(Matrix4x4));
	// データを書き込む
	Matrix4x4* wvpData = nullptr;
	// 書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	// 単位行列を書き込んでおく
	*wvpData = MakeIdentity4x4();

	Microsoft::WRL::ComPtr<ID3D12Resource> transResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	TransformationMatrix* transData = nullptr;
	transResource->Map(0, nullptr, reinterpret_cast<void**>(&transData));

	//*平行光源*//

	// 平行光源用リソース（定数バッファ）を作成
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	DirectionalLight* directionalLightData = nullptr;
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;
	directionalLightResource->Unmap(0, nullptr);


	//*　インデックス　*//
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);

	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	// リソースの先頭のアドレスから使う
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	// 使用するリソースのサイズはindex６つ分のサイズ
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

	// インデックスリソースにデータを書き込む
	uint32_t* indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0;
	indexDataSprite[1] = 1;
	indexDataSprite[2] = 2;
	indexDataSprite[3] = 1;
	indexDataSprite[4] = 3;
	indexDataSprite[5] = 2;


	//*Sprite*//

#pragma region 基盤システム

	// ポインタ
	SpriteCommon* spriteCommon = nullptr;
	// スプライト共通部の初期化
	spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);

#pragma endregion

#pragma region 最初のシーン

	// ポインタ
	Sprite* sprite = nullptr;
	sprite = new Sprite();

#pragma endregion

	sprite->Initialize(spriteCommon,windowAPI,dxCommon);

	// Sprite用の頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = dxCommon->CreateBufferResource(sizeof(VertexData) * 6);

	// 頂点バッファビューを作成
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	// リソースの先頭のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点６つの分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	// 1頂点あたりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	// 頂点データを設定する
	VertexData* vertexDataSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	// １枚目の三角形
	vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };// 左下
	vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	vertexDataSprite[0].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };// 左上
	vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	vertexDataSprite[1].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };// 右下
	vertexDataSprite[2].texcoord = { 1.0f,1.0f };
	vertexDataSprite[2].normal = { 0.0f,0.0f,-1.0f };
	// 2枚目の三角形
	vertexDataSprite[3].position = { 640.0f,0.0f,0.0f,1.0f };// 右上
	vertexDataSprite[3].texcoord = { 1.0f,0.0f };
	vertexDataSprite[3].normal = { 0.0f,0.0f,-1.0f };

	// Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 １つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = dxCommon->CreateBufferResource(sizeof(Matrix4x4));
	// データを書き込む
	Matrix4x4* transformationMatrixDataSprite = nullptr;
	// 書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	// 単位行列を書き込んでおく
	*transformationMatrixDataSprite = MakeIdentity4x4();

	// マテリアル用のリソースを作る。今回はcolor１つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = dxCommon->CreateBufferResource(sizeof(Material));
	// マテリアルにデータを書き込む
	Material* materialDataSprite = nullptr;
	// 書き込むためのアドレスを取得
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));

	// 色を設定する。Spriteは白色で表示する
	materialDataSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	// SpriteはLightingしないのでfalseを設定する
	materialDataSprite->enableLighting = false;
	// UVTransform行列
	materialDataSprite->uvTransform = MakeIdentity4x4();

	
	// TextureをtextureResource 読んで転送
	DirectX::ScratchImage mipImages = dxCommon->LoadTexture("Resource/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = dxCommon->CreateTextureResource(metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = dxCommon->UploadTextureData(textureResource, mipImages);

	// metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	// SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxCommon->srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxCommon->srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	// 先頭はImGuiを使っているのでその次を使う
	textureSrvHandleCPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);



	//*2枚目のテクスチャ*//

	// 2枚目のTextureを読んで転送する
	DirectX::ScratchImage mipImages2 = dxCommon->LoadTexture(modelData.material.textureFilePath);
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = dxCommon->CreateTextureResource(metadata2);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource2 = dxCommon->UploadTextureData(textureResource2, mipImages2);

	// 2枚目meataDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;// 2Dテクスチャ
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	// SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = dxCommon->GetSRVCPUDescriptorHandle(2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = dxCommon->GetSRVGPUDescriptorHandle(2);
	// SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);

	// デバックカメラ
	DebugCamera* debugCamera = new DebugCamera();
	// カメラの初期化
	debugCamera->Initialize();


	// 音声読み込み
	SoundData soundData1 = SoundLoadWave("Resource/Alarm01.wav");
	// 音声再生
	//SoundPlayWave(xAudio2, soundData1);

	MSG msg{};
	// ウィンドウのｘボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		// Windowsのメッセ維持処理
		if (windowAPI->ProcessMessage()) {
			// ゲームループを抜ける
			break;
		}

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		// ゲームの処理

		// 入力の更新
		input->Update();

		// デバックカメラ
		debugCamera->Update(windowAPI->GetHwnd());

		// 数字の０キーが押されていたら
		if (input->TriggerKey(DIK_0)) {
			OutputDebugStringA("Hit 0\n"); // 出力ウィンドウに「Hit ０」と表示
		}

		// y軸回転処理
		transform.rotate.y = 3.00f;

		Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		viewMatrix = debugCamera->GetViewMatrix(); // デバッグカメラのビュー行列を取得
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(windowAPI->kClientWidth) / float(windowAPI->kClientHeight), 0.1f, 100.0f);
		// WVPmatrixを作る
		Matrix4x4 worldViewProjectionMatrix = Multiply(Multiply(worldMatrix, viewMatrix), projectionMatrix);
		*wvpData = worldViewProjectionMatrix;
		transData->WVP = worldViewProjectionMatrix;   // WVP行列を設定
		transData->World = worldMatrix; // World行列を設定


		// *スプライト* //

		// 座標
		Vector2 position = sprite->GetPosition();
		position += Vector2{ 0.1f,0.1f };
		sprite->SetPosition(position);
		// 回転
		float rotation = sprite->GetRotation();
		rotation += 0.01f;
		sprite->SetRotation(rotation);
		// 色
		Vector4 color = sprite->GetColor();
		color.x += 0.01f;
		if (color.x > 1.0f) {
			color.x -= 1.0f;
		}
		sprite->SetColor(color);

		// sprite更新
		sprite->Update();
		

		// これから書き込むバックバッファのインデックスを取得

		// スライダー
		//UI
		ImGui::SliderFloat("SpritePosX", &tranaformSprite.translate.x, 0.0f, 500.0f);
		ImGui::SliderFloat("SpritePosY", &tranaformSprite.translate.y, 0.0f, 500.0f);

		// ライトの向き
		ImGui::SliderFloat("directionX", &directionalLightData->direction.x, -10.0f, 10.0f);
		ImGui::SliderFloat("directionY", &directionalLightData->direction.y, -10.0f, 10.0f);
		ImGui::SliderFloat("directionZ", &directionalLightData->direction.z, -10.0f, 10.0f);

		// SRVの切り替え
		ImGui::Checkbox("UseMonsterBall", &useMonsterBall);

		// UV座標
		ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);

		// モデル
		ImGui::DragFloat3("scale", &transform.scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("translate", &transform.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("rotate", &transform.rotate.x, 0.01f, -10.0f, 10.0f);



		// UVTransform用の行列を生成
		Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
		materialDataSprite->uvTransform = uvTransformMatrix;

		// 描画前処理
		dxCommon->PreDraw();



		// Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
		spriteCommon->SetCommonPipelineState();

		// モデル
		// RootSignatureを設定。PSOに設定しているけど別途設定が必要
		dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定
		
		// マテリアルCBufferの場所を設定
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		// wvp用とWorld用のCBufferの場所を設定
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transResource->GetGPUVirtualAddress());
		// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
		dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
		// 平行光源
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

		// インデックスバッファビューを設定
		dxCommon->GetCommandList()->IASetIndexBuffer(&indexBufferViewVertex);
		// インデックスを使って描画（球）
		dxCommon->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

		// スプライト描画
		sprite->Draw();

		// 実際のcommandListのImGuiの描画コマンドを詰む
		ImGui::Render();
		if (ImDrawData* draw_data = ImGui::GetDrawData()) {
			ImGui_ImplDX12_RenderDrawData(draw_data, dxCommon->GetCommandList());
		}

		dxCommon->PostDraw();


	}

	// ImGuiの終了処理。詳細はさして重要ではないので解説は省略する。
	//　こういうもんである。初期化と逆順に行う
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// 解放
	CloseHandle(dxCommon->fenceEvent);

	// 音声データ解放
	//xAudio2.Reset();

	// 入力の初期化
	delete input;
	// WindowAPIの終了処理
	windowAPI->Finalize();
	// WindowAPIの解放
	delete windowAPI;
	// DirectX解放
	delete dxCommon;
	// スプライト解放
	delete sprite;
	delete spriteCommon;

	CoUninitialize();

	return 0;

}
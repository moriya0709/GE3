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

#include "Calc.h"
#include "Input.h"
#include "WindowAPI.h"
#include "DirectXCommon.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "ObjectCommon.h"
#include "Object.h"

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


	// テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(dxCommon);
	
	

#pragma region 基盤システム

	// スプライト共通部
	SpriteCommon* spriteCommon = nullptr;
	// スプライト共通部の初期化
	spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);

	// 3dスプライト共通部
	ObjectCommon* objectCommon = nullptr;
	// 3dスプライト共通部の初期化
	objectCommon = new ObjectCommon();
	objectCommon->Initialize(dxCommon);

#pragma endregion

#pragma region 最初のシーン

	// スプライト
	Sprite* sprite = nullptr;
	sprite = new Sprite();
	sprite->Initialize(spriteCommon, windowAPI, dxCommon, "Resource/uvChecker.png");

	// 3Dオブジェクト
	Object* object = nullptr;
	object = new Object();
	object->Initialize(objectCommon,windowAPI,dxCommon);

	
#pragma endregion

	

	
	
	


	

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


		// 数字の０キーが押されていたら
		if (input->TriggerKey(DIK_0)) {
			OutputDebugStringA("Hit 0\n"); // 出力ウィンドウに「Hit ０」と表示
			// テクスチャ変更
			sprite->ChangeTexture("Resource/uvChecker.png");
		}

		// y軸回転処理
		transform.rotate.y = 3.00f;

		// * 3Dオブジェクト* //
		object->Update();

		// *スプライト* //

		// sprite更新
		sprite->Update();
		

		// これから書き込むバックバッファのインデックスを取得

		// スライダー
		//UI
		ImGui::SliderFloat("SpritePosX", &tranaformSprite.translate.x, 0.0f, 500.0f);
		ImGui::SliderFloat("SpritePosY", &tranaformSprite.translate.y, 0.0f, 500.0f);

		// ライトの向き
		//ImGui::SliderFloat("directionX", &directionalLightData->direction.x, -10.0f, 10.0f);
		//ImGui::SliderFloat("directionY", &directionalLightData->direction.y, -10.0f, 10.0f);
		//ImGui::SliderFloat("directionZ", &directionalLightData->direction.z, -10.0f, 10.0f);

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



		
		// 描画前処理
		dxCommon->PreDraw();

		// 3Dオブジェクトの描画準備
		objectCommon->SetCommonPipelineState();

		// 3Dオブジェクト描画
		object->Draw();

		// スプライトの描画準備
		spriteCommon->SetCommonPipelineState();

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
	// テクスチャマネージャの終了
	TextureManager::GetInstance()->Finalize();
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
	// 3Dオブジェクト解放
	delete object;
	delete objectCommon;

	CoUninitialize();

	return 0;

}
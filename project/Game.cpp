#include "Game.h"

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

void Game::Initialize() {
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
	WindowAPI* windowAPI = new WindowAPI();
	windowAPI->Initialize();

	// DirectXの初期化
	DirectXCommon* dxCommon = new DirectXCommon();
	dxCommon->Initialize(windowAPI);

	// Input初期化
	Input* input = new Input();
	input->Initialize(windowAPI);


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

	// カメラ初期化
	Camera* camera = new Camera();
	camera->SetRotate({ cameraTransform.rotate });
	camera->SetTranslate({ cameraTransform.translate });

	// カメラマネージャ登録
	auto* cameraManager = CameraManager::GetInstance();
	cameraManager->AddCamera("main", camera);
	cameraManager->SetActiveCamera("main");

	// カメラを3Dオブジェクト共通部にセット
	objectCommon->SetDefaultCamera(camera);

	// SRVマネージャ
	SrvManager* srvManager = nullptr;
	srvManager = new SrvManager();
	srvManager->Initialize(dxCommon);

	// テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(dxCommon, srvManager);

	// 3Dモデルマネージャの初期化
	ModelManager::GetInstance()->Initialize(dxCommon);

	// Particleマネージャ
	ParticleManager::GetInstance()->Initialize(dxCommon, srvManager, camera, "Resource/", "plane.obj");
	ParticleManager::GetInstance()->CreateParticleGroup("group1", "Resource/particle.png");
	ParticleManager::GetInstance()->CreateParticleGroup("group2", "Resource/uvChecker.png");

#pragma endregion

#pragma region 最初のシーン

	// スプライト
	Sprite* sprite = nullptr;
	sprite = new Sprite();
	sprite->Initialize(spriteCommon, dxCommon, "Resource/uvChecker.png");

	// 3Dオブジェクト
	Object* object[2]{};
	for (int i = 0; i < 2; i++) {
		object[i] = new Object();
		object[i]->Initialize(objectCommon, windowAPI, dxCommon);
	}

	// Emitパーティクル発生
	ParticleEmitter* particleEmitter = new ParticleEmitter();
	particleEmitter->Initialize("group1", transformParticle, 5, 1.0f);
	particleEmitter->Emit();

	// .objファイルからモデル読み込み
	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("axis.obj");

	// 初期化済みの3Dオブジェクトにモデルを紐づける
	object[0]->SetModel("plane.obj");
	object[1]->SetModel("axis.obj");

	// サウンド
	SoundManager* soundManager = new SoundManager();
	soundManager->Initialize();

	// 音声読み込み
	SoundData soundData1 = soundManager->SoundLoadFile("game.mp3");
	// 音声再生
	soundManager->SoundPlayWave(soundData1);

	// ImGui
	ImGuiManager* imGuiManager = new ImGuiManager();
	imGuiManager->Initialize(windowAPI, dxCommon, srvManager);

#pragma endregion

}

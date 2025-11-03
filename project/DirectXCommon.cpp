#include "DirectXCommon.h"

#include <cassert>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

using namespace Microsoft::WRL;

void DirectXCommon::Initialize(WindowAPI* windowAPI) {
    // NULL検出
    assert(windowAPI);
    // メンバ変数に記録
    this->windowAPI_ = windowAPI;

    CreateDevice(); // デバイス関連
    CreateCommandList(); // コマンドリスト関連
    CreateSwapChain(); // スワップチェイン関連
    CreateDepth(); // 深度バッファ関連
    CreateDescriptor(); // デスクリプタヒープ関連
    CreateDxcCompiler(); // DXCコンパイラの生成

    InitializeRTV(); // レンダーターゲットビューの初期化
    InitializeDSV(); // 深度ステンシルビューの初期化
    InitializeFence(); // フェンスの初期化
    InitializeViewport(); // ビューポート矩形の初期化
    InitializeScissorRect(); // シザリング矩形の初期化
    InitializeImGui(); // ImGuiの初期化
  

}

// コマンドリスト関連
void DirectXCommon::CreateCommandList() {
    // コマンドキューを生成する
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    HRESULT hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
    // コマンドキューの生成がうまくいかなかったので起動できない
    assert(SUCCEEDED(hr));

    // コマンドアロケータを生成する
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
    hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
    // コマンドアロケータの生成がうまくいかなかったので起動できない
    assert(SUCCEEDED(hr));

    // コマンドリストを生成する
    Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList = nullptr;
    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
    // コマンドリストの生成が上手くいかなかったので起動できない
    assert(SUCCEEDED(hr));

    // コマンドリスト閉じる
    commandList->Close();
}

// スワップチェイン関連
void DirectXCommon::CreateSwapChain() {
    HRESULT hr;
    
    // スワップチェイン生成の設定
    swapChainDesc.Width = windowAPI_->kClientWidth; // 画面の幅。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc.Height = windowAPI_->kClientHeight; // 画面の高さ。ウィンドウのクライアント小域を同じものにしておく
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 色の形式
    swapChainDesc.SampleDesc.Count = 1; // マルチサンプルしない
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //描画のターゲットとして利用する
    swapChainDesc.BufferCount = 2; // ダブルバッファ
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // モニタにうつしたら、中身を破棄

    // スワップチェイン生成
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    swapChain1.Reset();

    hr = dxgiFactory->CreateSwapChainForHwnd(
        commandQueue.Get(), windowAPI_->GetHwnd(), &swapChainDesc,
        nullptr, nullptr, swapChain1.GetAddressOf());
    assert(SUCCEEDED(hr));

    hr = swapChain1.As(&swapChain);
    assert(SUCCEEDED(hr));

    // SwapChainからResourceを引っ張ってくる
    Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2] = { nullptr };
    hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
    // 上手く取得できなければ起動できない
    assert(SUCCEEDED(hr));
    hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
    assert(SUCCEEDED(hr));
}

// 深度バッファ関連
void DirectXCommon::CreateDepth() {
   
    // DepthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    // Depthの機能を有効化する
    depthStencilDesc.DepthEnable = true;
    // 書き込みします
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    // 比較関数はLessEqual。つまり、近ければ描画される
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    // depthStencilリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = CreatDepthStenCilTextureResource(device, windowAPI_->kClientWidth, windowAPI_->kClientHeight);

}

// デスクリプタヒープ生成
void DirectXCommon::CreateDescriptor() {
    // DescriptorSizeを取得しておく
    descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    // RTV用のヒープディスクリプタの数は２。RTVはShader内で触るものではないので、ShaderVisibleはfalse
    rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
    // SRV用のヒープでディスクリプタの数は128。SRVはShader内で触るものなので、ShaderVisibleはtrue
    srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
    // DSV用のヒープでディスクリプタの数は１。DSVはShader内で触るものではないので、ShaderVicibleはfalse
    dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

}

// DXCコンパイラの生成
void DirectXCommon::CreateDxcCompiler() {
    // dxcCompilerを初期化
    IDxcUtils* dxcUtils = nullptr;
    IDxcCompiler3* dxcCompiler = nullptr;
    HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
    assert(SUCCEEDED(hr));
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
    assert(SUCCEEDED(hr));

    // 現時点でincludeはしないが、includeに対応するための設定を行っておく
    IDxcIncludeHandler* includeHandler = nullptr;
    hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
    assert(SUCCEEDED(hr));
}

// 深度バッファ用リソース生成
Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreatDepthStenCilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device>& device, int32_t width, int32_t height) {
    // 生成するResourceの設定
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width = width; // Textureの幅
    resourceDesc.Height = height; // Textureの高さ
    resourceDesc.MipLevels = 1; // mipmapの数
    resourceDesc.DepthOrArraySize = 1; // 奥行 or 配列Textureの配列数
    resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DepthStencilとして利用可能なフォーマット
    resourceDesc.SampleDesc.Count = 1; // サンプリングカウント。１固定
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知

    // 利用するHeapの設定
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

    // 深度値のクリア設定
    D3D12_CLEAR_VALUE depthClearValue = {};
    depthClearValue.DepthStencil.Depth = 1.0f; // 1.0f（最大値）でクリア
    depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // フォーマット。Resourceとア合わせる

    // Resourceの作成
    Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties, // Heapの設定
        D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定。特になし
        &resourceDesc, // Resourceの設定
        D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度値を書き込む状態にしておく
        &depthClearValue, // Clear最適値
        IID_PPV_ARGS(&resource)); // 作成するResourceポインタへのポインタ
    assert(SUCCEEDED(hr));

    return resource;
}

// デバイスの生成
void DirectXCommon::CreateDevice() {
    HRESULT hr;

#ifdef _DEBUG
    Microsoft::WRL::ComPtr <ID3D12Debug1> debugController = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        // デバックレイヤーを有効化する
        debugController->EnableDebugLayer();
        // さらにGPU側でもチェックを行うようにする
        debugController->SetEnableGPUBasedValidation(TRUE);
    }
#endif

    //DXGIファクトリーの生成
    hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
    assert(SUCCEEDED(hr)); // 失敗したら止める

    // 使用するアダプタ用の変数。最初にnullptrを入れておく
    Microsoft::WRL::ComPtr <IDXGIAdapter4> useAdapter = nullptr;
    // 良い順にアダプタを頼む
    for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
        DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter))
        != DXGI_ERROR_NOT_FOUND; ++i) {
        // アダプターの情報を取得する
        DXGI_ADAPTER_DESC3 adapterDesc{};
        hr = useAdapter->GetDesc3(&adapterDesc);
        assert(SUCCEEDED(hr)); // 取得できないのは一大事
        // ソフトウェアアダプタでなければ採用！
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
            // 採用したアダプタの情報をログに出力。wsrtingの方なので注意
            Log(ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
            break;
        }
        useAdapter = nullptr; // ソフトウェアアダプタの場合は見なかったことにする
    }
    // 適切なアダプタが見つからなかったので起動できない
    assert(useAdapter != nullptr);

    device = nullptr;
    // 昨日レベルとログ出力用の文字列
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_2,
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };
    const char* featureLevelStrings[] = { "12.2","12.1","12.0","11.1","11.0" };
    // 高い順に生成できるか試していく
    for (size_t i = 0; i < _countof(featureLevels); ++i) {
        // 採用したアダプターでデバイスを生成
        hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
        // 指定した機能レベルでデバイスが生成できたかを確認
        if (SUCCEEDED(hr)) {
            // 生成できたのでログ出力を行ってループを抜ける
            Log(std::format("Featurelevel : {}\n", featureLevelStrings[i]));
            break;
        }
    }
    // デバイスの生成が上手くいかなかったので起動できない
    assert(device != nullptr);
    Log("Complete create D3D12Device!!!\n");

#ifdef _DEBUG
    Microsoft::WRL::ComPtr <ID3D12InfoQueue> infoQueue = nullptr;
    if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
        // ヤバいエラー時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        // エラー時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        // 警告時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

        // 抑制するメッセージのID
        D3D12_MESSAGE_ID denyIds[] = {
            // Windows11でのDXGIデバックレイヤーとDX12デバックレイヤーの相互作用バグによるエラーメッセージ
            // https://stackoverflow.com/questions/69805245/direcctx-12-application-is-crashing-in-windows-11
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
        };
        // 抑制するレベル
        D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
        D3D12_INFO_QUEUE_FILTER filter{};
        filter.DenyList.NumIDs = _countof(denyIds);
        filter.DenyList.pIDList = denyIds;
        filter.DenyList.NumSeverities = _countof(severities);
        filter.DenyList.pSeverityList = severities;
        // 指定したメッセージの表示を抑制する
        infoQueue->PushStorageFilter(&filter);

    }
#endif
}

// レンダーターゲットビューの初期化
void DirectXCommon::InitializeRTV() {
    // SwapChainからResourceを引っ張ってくる
    Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2] = { nullptr };
    HRESULT hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
    // 上手く取得できなければ起動できない
    assert(SUCCEEDED(hr));
    hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
    assert(SUCCEEDED(hr));

    // RTVの設定
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 出力結果をSRGBに変換して書き込む
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // 2dテクスチャとして書き込む
    // ディスクリプタの先頭を取得する
    D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    // RTVを２つ作るのでディスクリプタを２つ用意
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
   
    // ディスクリプタハンドルを得る
    rtvHandles[0] = rtvStartHandle;
    rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // 裏表の２つ分
    for (uint32_t i = 0; i < 2; ++i) {
        // RTVの生成
        device->CreateRenderTargetView(swapChainResources[i].Get(), &rtvDesc, rtvHandles[i]);
    }

}

// 深度ステンシルビューの初期化
void DirectXCommon::InitializeDSV() {
    // DSVの設定
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Format。基本的にはResourceに合わせる
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2dTexture
    
    // DSSVHeapの先頭にDSVをつくる
    device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());


}

void DirectXCommon::InitializeFence() {
    // 初期値０でFenceを作る
    Microsoft::WRL::ComPtr <ID3D12Fence> fence = nullptr;
    uint64_t fenceValue = 0;
    HRESULT hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    assert(SUCCEEDED(hr));

    // FenceのSignalを松ためのイベントを作成する
    HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent != nullptr);
}

void DirectXCommon::InitializeViewport() {
    // クライアント領域のサイズと一緒にして画面全体に表示
    viewport.Width = windowAPI_->kClientWidth;
    viewport.Height = windowAPI_->kClientHeight;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
}

void DirectXCommon::InitializeScissorRect() {
    // 基本的にビューポートと同じ矩形が構成されるようにする
    scissorRect.left = 0;
    scissorRect.right = windowAPI_->kClientWidth;
    scissorRect.top = 0;
    scissorRect.bottom = windowAPI_->kClientHeight;
}

void DirectXCommon::InitializeImGui() {
    // ImGuiの初期化
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(windowAPI_->GetHwnd());
    ImGui_ImplDX12_Init(device.Get(),
        swapChainDesc.BufferCount,
        rtvDesc.Format,
        srvDescriptorHeap.Get(),
        srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
}

// デスクリプタヒープ生成
Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device>& device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {
    Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap = nullptr;
    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
    descriptorHeapDesc.Type = heapType;
    descriptorHeapDesc.NumDescriptors = numDescriptors;
    descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
    assert(SUCCEEDED(hr));
    return descriptorHeap;
}

// SRVの指定番号のCPUデスクリプタハンドルを取得
D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVCPUDescriptorHandle(uint32_t index) {
    return GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, index);
}

// SRVの指定番号のGPUデスクリプタハンドルを取得
D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVGPUDescriptorHandle(uint32_t index) {
    return D3D12_GPU_DESCRIPTOR_HANDLE();
}

std::string DirectXCommon::ConvertString(const std::wstring& str) {
    if (str.empty()) {
        return std::string();
    }

    auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
    if (sizeNeeded == 0) {
        return std::string();
    }
    std::string result(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
    return result;
}

// 指定番号のCPUデスクリプタハンドルを取得
D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index) {
    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += (descriptorSize * index);
    return handleCPU;
}
// 指定番号のGPUデスクリプタハンドルを取得
D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index) {
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += (descriptorSize * index);
    return handleGPU;
}


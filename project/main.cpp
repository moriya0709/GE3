#define _USE_MATH_DEFINES
#define DIRECTINPUT_VERSION 0x0800

#include <Windows.h>
#include <cstdint>
#include <string>1
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

#include "DebugCamera.h"
#include "Calc.h"
#include "Input.h"
#include "WindowAPI.h"

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


//��������i�[����
std::string str0{ "STRING" };

//���O�̃f�B���N�g����p��
namespace fs = std::filesystem;

// 3x3�s��
struct Matrix3x3 {
	float m[3][3] = { 0 };
};


// Transform
struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};


// transform�̏�����
Transform transform
{
	{ 1.0f, 1.0f, 1.0f }, // scale
	{ 0.0f, 0.0f, 0.0f }, // rotate
	{ 0.0f, 0.0f, 0.0f }  // translate
};

// transformSprite�̏�����
Transform tranaformSprite
{
	{1.0f,1.0f,1.0f},
	{0.0f,0.0f,0.0f},
	{0.0f,0.0f,0.0f}
};


// cameraTransform�̏�����
Transform cameraTransform
{
	{ 1.0f, 1.0f, 1.0f }, // scale
	{ 0.0f, 0.0f, 0.0f }, // rotate
	{ 0.0f, 0.0f, -5.0f } // translate
};

// ���_�f�[�^
struct VertexData {
	Vector4 position; // ���_���W
	Vector2 texcoord; // �e�N�X�`�����W
	Vector3 normal; // ���K�����W
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
	Vector4 color; // ���C�g�̐F
	Vector3 direction; // ���C�g�̌���
	float intensity; // �P�x
};

struct MaterialData {
	std::string textureFilePath;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

// �`�����N�w�b�_
struct ChunkHeader {
	char id[4]; // �`�����N����ID
	int32_t size; // �`�����N�T�C�Y
};

// �t�H�[�}�b�g�`�����N
struct FormatChunk {
	ChunkHeader chunk; // "fmt "�`�����N�w�b�_�[
	WAVEFORMATEX  fmt; // �t�H�[�}�b�g�{�́i�ő�40�o�C�g���x�j
};

// RIFF�w�b�_�`�����N
struct RiffHeader {
	ChunkHeader chunk; // RIFF
	char type[4]; // WAVE
};

// �����f�[�^
struct SoundData {
	// �g�`�t�H�[�}�b�g
	WAVEFORMATEX wfex;
	// �o�b�t�@�̐擪�A�h���X
	BYTE* pBuffer;
	// �o�b�t�@�̃T�C�Y
	unsigned int bufferSize;
};


Transform uvTransformSprite{
	{1.0f,1.0f,1.0f},
	{0.0f,0.0f,0.0f},
	{0.0f,0.0f,0.0f},
};

// SRV�؂�ւ�
bool useMonsterBall = true;

// �P�ʍs��̍쐬
Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result = {}; // �[��������
	for (int i = 0; i < 4; ++i)
		result.m[i][i] = 1.0f;
	return result;
}

static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Duumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	// processId(����exe��ID)�ƃN���b�V���i��O�j�̔�������threadId���擾
	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();
	// �ݒ�������
	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
	minidumpInformation.ThreadId = threadId;
	minidumpInformation.ExceptionPointers = exception;
	minidumpInformation.ClientPointers = TRUE;
	//Dump���o�́BMiniDumpNormal�͍Œ���̏����o�͂���v���O
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

std::string ConvertString(const std::wstring& str) {
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

// string �� wstring (UTF-8�ϊ�)
std::wstring ConvertString(const std::string& str) {
	if (str.empty()) return std::wstring();

	int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), NULL, 0);
	if (sizeNeeded == 0) return std::wstring();

	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &result[0], sizeNeeded);
	return result;
}

// �E�B���h�E�v���V�[�W��
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) {
		return true;
	}

	// ���b�Z�[�W�ɉ����ăQ�[���ŗL�̏������s��
	switch (msg) {
		// �E�B���h�E���j�󂳂ꂽ
	case WM_DESTROY:
	// OS�ɑ΂��āA�A�v���̏I����`����
	PostQuitMessage(0);
	return 0;
	}
	// �W���̃��b�Z�[�W�������s��
	return DefWindowProc(hwnd, msg, wParam, lParam);

}

IDxcBlob* CompileShader(
	// Compiler����Shader�t�@�C���ւ̃p�X
	const std::wstring& filePath,
	// Compiler�Ɏg�p����Profile
	const wchar_t* profile,
	// �������Ő����������̂��R��
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler,
	std::ostream& os) {

	// ���ꂩ��V�F�[�_�[���R���p�C������|�����O�ɏo��
	Log(os, ConvertString(std::format(L"Begin CompileShader, path:{},profile:{}\n", filePath, profile)));
	// hlsl�t�@�C����ǂ�
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	// �ǂ߂Ȃ�������~�߂�
	assert(SUCCEEDED(hr));
	// �ǂݍ��񂾃t�@�C���̓��e��ݒ肷��
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	LPCWSTR arguments[] = {
	filePath.c_str(), // �R���p�C���Ώۂ�hlsl�t�@�C����
	L"-E", L"main", // �G���g���[�|�C���g�̎w��B��{�I��main�ȊO�ɂ͂��Ȃ�
	L"-T",profile, // shaderProfile�̐ݒ�
	L"-Zi",L"-Qembed_debug", // �f�o�b�O�p�̏��𖄂ߍ���
	L"-Od", //�œK�����O���Ă���
	L"-Zpr", // �������[���C�A�E�g�͍s�D��

	};
	// ���ۂ�Shader���R���p�C������
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer, // �ǂݍ��񂾃t�@�C��
		arguments, // �R���p�C���I�v�V����
		_countof(arguments), // �R���p�C���I�v�V�����̐�
		includeHandler, // include���܂܂ꂽ���X
		IID_PPV_ARGS(&shaderResult) // �R���p�C������
	);
	// �R���p�C���G���[�ł͂Ȃ�dxc���N���ł��Ȃ��Ȃǒv���I�ȏ�
	assert(SUCCEEDED(hr));

	// �x���E�G���[���łĂ����烍�O�ɏo���Ď~�߂�
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		// �x���E�G���[�_���[�b�^�C
		assert(false);
	}

	// �R���p�C�����ʂ�����s�p�̃o�C�i���������擾
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	//�����������O���o��
	Log(ConvertString(std::format(L"Compile Succeeded,path:{},profile;{}\n", filePath, profile)));
	// �����g��Ȃ����\�[�X�����
	//shaderSource->Release();
	//shaderResult->Release();
	// ���s�p�̃o�C�i����ԋp
	return shaderBlob;
}

// Resource�쐬
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device>& device, size_t sizeInBytes) {
	// 256�o�C�g�P�ʂɐ؂�グ
	const UINT alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT; // 256
	sizeInBytes = (sizeInBytes + alignment - 1) & ~(alignment - 1);

	D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr));
	return resource;
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

// Texture�f�[�^��ǂݍ���
DirectX::ScratchImage LoadTexture(const std::string& filePath) {
	// �e�N�X�`���t�@�C����ǂ�Ńv���O�����ň�����悤�ɂ���
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// �~�b�v�}�b�v�̍쐬
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// �~�b�v�}�b�v�t���Ƀf�[�^��Ԃ�
	return mipImages;
}

// TextureResource���쐬����
Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device>& device, const DirectX::TexMetadata& metadata) {
	// metadata�����Resource�̐ݒ�
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width); // Texture�̕�
	resourceDesc.Height = UINT(metadata.height); // Texture�̍���
	resourceDesc.MipLevels = UINT16(metadata.mipLevels); // mipamap�̐�
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize); // ���s�@or�@�z��Texture�̔z��
	resourceDesc.Format = metadata.format; // Texture�̃t�H�[�}�b�g
	resourceDesc.SampleDesc.Count = 1; // �T���v�����O�J�E���g�B1�Œ�
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension); // Texture�̎�����

	// ���p����Heap�̐ݒ�B���ɓ���ȉ^�p�B02_04ex�ň�ʓI�ȃP�[�X�ł�����
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // �ׂ����ݒ���s��
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; // WriteBack�|���V�[��CPU�A�N�Z�X�\
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; // �v���Z�b�T�[�̋߂��ɔz�u

	// Resource�̐���
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties, // Heap�̐ݒ�
		D3D12_HEAP_FLAG_NONE, // Heap�̓���Ȑݒ�B
		&resourceDesc, // Resource�̐ݒ�
		D3D12_RESOURCE_STATE_COPY_DEST, // �����ResourceState�BTexture�͊�{�ǂނ���
		nullptr, // Clear�œK�l�B�g��Ȃ��̂�nullptr
		IID_PPV_ARGS(&resource)); // �쐬����Resource�̃|�C���^�ւ̃|�C���^
	assert(SUCCEEDED(hr));
	return resource;
}

// �f�[�^��]������
[[nodiscard]]
Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages, Microsoft::WRL::ComPtr<ID3D12Device>& device, Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList) {
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subresources.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(device, intermediateSize);
	UpdateSubresources(commandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, UINT(subresources.size()), subresources.data());
	// Teture�ւ̓]����͗��p�ł���悤�AD3D12_RESOURCE_STATE_COPY_DEST����D3D12_RESOURCE_STATE_GENERIC_READ��ResourceState��ύX����
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);
	return intermediateResource;
}

// �[�x�o�b�t�@�p���\�[�X����
Microsoft::WRL::ComPtr<ID3D12Resource> CreatDepthStenCilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device>& device, int32_t width, int32_t height) {
	// ��������Resource�̐ݒ�
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width; // Texture�̕�
	resourceDesc.Height = height; // Texture�̍���
	resourceDesc.MipLevels = 1; // mipmap�̐�
	resourceDesc.DepthOrArraySize = 1; // ���s or �z��Texture�̔z��
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DepthStencil�Ƃ��ė��p�\�ȃt�H�[�}�b�g
	resourceDesc.SampleDesc.Count = 1; // �T���v�����O�J�E���g�B�P�Œ�
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2����
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencil�Ƃ��Ďg���ʒm

	// ���p����Heap�̐ݒ�
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM��ɍ��

	// �[�x�l�̃N���A�ݒ�
	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.DepthStencil.Depth = 1.0f; // 1.0f�i�ő�l�j�ŃN���A
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // �t�H�[�}�b�g�BResource�ƃA���킹��

	// Resource�̍쐬
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties, // Heap�̐ݒ�
		D3D12_HEAP_FLAG_NONE, // Heap�̓���Ȑݒ�B���ɂȂ�
		&resourceDesc, // Resource�̐ݒ�
		D3D12_RESOURCE_STATE_DEPTH_WRITE, // �[�x�l���������ޏ�Ԃɂ��Ă���
		&depthClearValue, // Clear�œK�l
		IID_PPV_ARGS(&resource)); // �쐬����Resource�|�C���^�ւ̃|�C���^
	assert(SUCCEEDED(hr));

	return resource;
}

#include <cmath>

// ���K���֐�
void Normalize(float& x, float& y, float& z) {
	float len = std::sqrt(x * x + y * y + z * z);
	if (len > 0.00001f) {
		x /= len;
		y /= len;
		z /= len;
	}
}

// ����`�悷��֐�
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

			// �O�p�`1: a, b, c
			vertexData[start + 0] = a;
			vertexData[start + 1] = b;
			vertexData[start + 2] = c;

			// �O�p�`2: c, b, d
			vertexData[start + 3] = d;

		}
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHaep, uint32_t descriptorSize, uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHaep->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHaep, uint32_t descriptorSize, uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHaep->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

// mtl�t�@�C����ǂފ֐�
MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	MaterialData materialData; // �\�z����MaterialData
	std::string line; // �t�@�C������ǂ񂾂P�s���i�[�������
	std::ifstream file(directoryPath + "/" + filename); // �t�@�C�����J��
	assert(file.is_open()); // �Ƃ肠���������Ȃ�������~�߂�

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifier�ɑ�H������
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			// �A�����ăt�@�C���p�X�ɂ���
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}

	}

	return materialData;
}

// obj�t�@�C����ǂފ֐�
ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData; // �\�z����ModelData
	std::vector<Vector4> positions; //�ʒu
	std::vector<Vector3> normals; // �@��
	std::vector<Vector2> texcoords; //�@�e�N�X�`�����W
	std::string line; // �t�@�C������ǂ�1�s���i�[�������

	std::ifstream file(directoryPath + "/" + filename); // �t�@�C�����J��
	assert(file.is_open()); // �Ƃ肠�����J���Ȃ�������~�߂�

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // �擪�̎��ʎq��ǂ�

		// identifier�ɉ���������
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
			// �ʂ͎O�p�`����B���̑��͖��Ή�
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				// ���_�̗v�f�ւ�Index�́u�ʒu/UV/�@���v�Ŋi�[����Ă���̂ŁA��������Index���擾����
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/'); // ��؂�ŃC���f�b�N�X��ǂ�ł���
					elementIndices[element] = std::stoi(index);
				}
				// �v�f�ւ�Index����A���ۂ̗v�f�̒l���擾���āA���_���\������
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				texcoord.y = 1.0f - texcoord.y;
				triangle[faceVertex] = { position,texcoord,normal };
			}

			// ���_���t���œo�^���邱�ƂŁA��菇���t�ɂ���
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		} else if (identifier == "mtllib") {
			// materialTemplateLibrary�t�@�C���̖��O���擾����
			std::string materialFilename;
			s >> materialFilename;
			// ��{�I��obj�t�@�C���Ɠ���K�w��mtl�͑��݂�����̂ŁA�f�B���N�g�����ƃt�@�C������n��
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}

	return modelData;
}

// �����f�[�^�̓ǂݍ���
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

	assert(dataHeader.size < 100 * 1024 * 1024); // 100MB�����ȂǓK�X

	char* pBuffer = new char[dataHeader.size];
	file.read(pBuffer, dataHeader.size);
	assert(file.gcount() == dataHeader.size);

	file.close();

	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = dataHeader.size;

	return soundData;
}

// �����f�[�^���
void SoundUnload(SoundData* soundData) {
	// �o�b�t�@�̃����������
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

// �����Đ�
void SoundPlayWave(Microsoft::WRL::ComPtr<IXAudio2> xAudio2, const SoundData& soundData) {
	HRESULT result;

	// �g�`�t�H�[�}�b�g������SourceVoice�̐���
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	// �Đ�����g�`�f�[�^�̐ݒ�
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// �g�`�f�[�^�̍Đ�
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();

}

// Windows�A�v���ł̃G���g���[�|�C���g(main�֐�)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	// �|�C���^
	Input* input = nullptr; // input
	WindowAPI* windowAPI = nullptr; // windowAPI



	// ���\�[�X���[�N�`�F�b�N
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


	// COM�̏�����
	CoInitializeEx(0, COINIT_MULTITHREADED);

	// �N���⑫���Ȃ������ꍇ��(Unhandled),�⑫����֐���o�^
	SetUnhandledExceptionFilter(ExportDump);

#ifdef _DEBUG
	Microsoft::WRL::ComPtr <ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		// �f�o�b�N���C���[��L��������
		debugController->EnableDebugLayer();
		// �����GPU���ł��`�F�b�N���s���悤�ɂ���
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif

	// WindowAPI�̏�����
	windowAPI = new WindowAPI();
	windowAPI->Initialize();

	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

	// ���ݎ������擾
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	// ���O�t�@�C���̖��O�ɃR���}���b�͂���Ȃ��̂ŁA����ĕb�ɂ���
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	// ���{���ԁiPC�̐ݒ莞��)�ɕϊ�
	std::chrono::zoned_time localTime{ std::chrono::current_zone(),nowSeconds };
	// format���g���ĔN����_�����b�̕�����ɕϊ�
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
	// �������g�ăt�@�C����������
	std::string logFilePath = std::string("logs/") + dateString + ".log";
	// �t�@�C�����g���ď������ݏ���
	std::ofstream logStream(logFilePath);

	// �o�̓E�B���h�E�ւ̕����o��
	OutputDebugStringA("Hello,DirectX!\n");

	// ���O
	bool logs = fs::create_directory("logs");
	Log(logStream, logFilePath);

	//DXGI�t�@�N�g���[�̐���
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	// HRESULT��Windows�n�̃G���[�R�[�h�ł���A
	// �֐��������������ǂ�����SUCCEEDED�}�N���Ŕ���ł���
	hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));

	//XAudio2�̏�����
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice = nullptr;
	HRESULT result;
	// XAudio�G���W���̃C���X�^���X�𐶐�
	result = XAudio2Create(xAudio2.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));
	// �}�X�^�[�{�C�X�𐶐�
	result = xAudio2->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(result));


	// �g�p����A�_�v�^�p�̕ϐ��B�ŏ���nullptr�����Ă���
	Microsoft::WRL::ComPtr <IDXGIAdapter4> useAdapter = nullptr;
	// �ǂ����ɃA�_�v�^�𗊂�
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter))
		!= DXGI_ERROR_NOT_FOUND; ++i) {
		// �A�_�v�^�[�̏����擾����
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr)); // �擾�ł��Ȃ��͈̂�厖
		// �\�t�g�E�F�A�A�_�v�^�łȂ���΍̗p�I
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			// �̗p�����A�_�v�^�̏������O�ɏo�́Bwsrting�̕��Ȃ̂Œ���
			Log(ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr; // �\�t�g�E�F�A�A�_�v�^�̏ꍇ�͌��Ȃ��������Ƃɂ���
	}
	// �K�؂ȃA�_�v�^��������Ȃ������̂ŋN���ł��Ȃ�
	assert(useAdapter != nullptr);

	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
	// ������x���ƃ��O�o�͗p�̕�����
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
	// �������ɐ����ł��邩�����Ă���
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		// �̗p�����A�_�v�^�[�Ńf�o�C�X�𐶐�
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
		// �w�肵���@�\���x���Ńf�o�C�X�������ł��������m�F
		if (SUCCEEDED(hr)) {
			// �����ł����̂Ń��O�o�͂��s���ă��[�v�𔲂���
			Log(std::format("Featurelevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	// �f�o�C�X�̐�������肭�����Ȃ������̂ŋN���ł��Ȃ�
	assert(device != nullptr);
	Log("Complete create D3D12Device!!!\n");

#ifdef _DEBUG
	Microsoft::WRL::ComPtr <ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		// ���o���G���[���Ɏ~�܂�
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// �G���[���Ɏ~�܂�
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		// �x�����Ɏ~�܂�
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		// �}�����郁�b�Z�[�W��ID
		D3D12_MESSAGE_ID denyIds[] = {
			// Windows11�ł�DXGI�f�o�b�N���C���[��DX12�f�o�b�N���C���[�̑��ݍ�p�o�O�ɂ��G���[���b�Z�[�W
			// https://stackoverflow.com/questions/69805245/direcctx-12-application-is-crashing-in-windows-11
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		// �}�����郌�x��
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		// �w�肵�����b�Z�[�W�̕\����}������
		infoQueue->PushStorageFilter(&filter);

		// ���
		//infoQueue->Release();
	}
#endif

	// DirectX�̏�����

	// Input������
	input = new Input();
	input->Initialize(windowAPI);


	// �R�}���h�L���[�𐶐�����
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	// �R�}���h�L���[�̐��������܂������Ȃ������̂ŋN���ł��Ȃ�
	assert(SUCCEEDED(hr));

	// �R�}���h�A���P�[�^�𐶐�����
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	// �R�}���h�A���P�[�^�̐��������܂������Ȃ������̂ŋN���ł��Ȃ�
	assert(SUCCEEDED(hr));

	// �R�}���h���X�g�𐶐�����
	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	// �R�}���h���X�g�̐�������肭�����Ȃ������̂ŋN���ł��Ȃ�
	assert(SUCCEEDED(hr));

	// �X���b�v�`�F�C���𐶐�����
	Microsoft::WRL::ComPtr <IDXGISwapChain4> swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = WindowAPI::kClientWidth; // ��ʂ̕��B�E�B���h�E�̃N���C�A���g�̈�𓯂����̂ɂ��Ă���
	swapChainDesc.Height = WindowAPI::kClientHeight; // ��ʂ̍����B�E�B���h�E�̃N���C�A���g����𓯂����̂ɂ��Ă���
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // �F�̌`��
	swapChainDesc.SampleDesc.Count = 1; // �}���`�T���v�����Ȃ�
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //�`��̃^�[�Q�b�g�Ƃ��ė��p����
	swapChainDesc.BufferCount = 2; // �_�u���o�b�t�@
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // ���j�^�ɂ�������A���g��j��

	// �X���b�v�`�F�C�����쐬
	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
	hr = dxgiFactory->CreateSwapChainForHwnd(
		commandQueue.Get(), windowAPI->GetHwnd(), &swapChainDesc,
		nullptr, nullptr, swapChain1.GetAddressOf());
	assert(SUCCEEDED(hr));

	hr = swapChain1.As(&swapChain);
	assert(SUCCEEDED(hr));

	// SwapChain����Resource�����������Ă���
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	// ��肭�擾�ł��Ȃ���΋N���ł��Ȃ�
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

	// RTV�p�̃q�[�v�f�B�X�N���v�^�̐��͂Q�BRTV��Shader���ŐG����̂ł͂Ȃ��̂ŁAShaderVisible��false
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	// SRV�p�̃q�[�v�Ńf�B�X�N���v�^�̐���128�BSRV��Shader���ŐG����̂Ȃ̂ŁAShaderVisible��true
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

	// RTV�̐ݒ�
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // �o�͌��ʂ�SRGB�ɕϊ����ď�������
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // 2d�e�N�X�`���Ƃ��ď�������
	// �f�B�X�N���v�^�̐擪���擾����
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	// RTV���Q���̂Ńf�B�X�N���v�^���Q�p��
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	// �܂���1�ڂ����B�P�ڂ͍ŏ��̂Ƃ���ɍ��B���ꏊ��������Ŏw�肵�Ă�����K�v������
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);
	// �Q�ڂ̃f�B�X�N���v�^�n���h���𓾂�i���͂Łj
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// �Q�ڂ����
	device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);

	// �����l�O��Fence�����
	Microsoft::WRL::ComPtr <ID3D12Fence> fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	// Fence��Signal�������߂̃C�x���g���쐬����
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);

	// dxcCompiler��������
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	// �����_��include�͂��Ȃ����Ainclude�ɑΉ����邽�߂̐ݒ���s���Ă���
	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));

	// DescriptorRange�쐬
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; // 0����n�܂�
	descriptorRange[0].NumDescriptors = 1; // ����1��
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRV���g��
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // offset�������v�Z

	// RootSignature�쐬
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// RootParameter�쐬
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBV���g��
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShader�Ŏg��
	rootParameters[0].Descriptor.ShaderRegister = 0; // ���W�X�^�ԍ��O�ƃo�C���h
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBV���g��
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // PixelShader�Ŏg��
	rootParameters[1].Descriptor.ShaderRegister = 0; // ���W�X�^�ԍ�0�ƃo�C���h
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTable���g��
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShader�Ŏg��
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange; // Table�̒��g�̔z����w��
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Table�ŗ��p���鐔
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBV���g��
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShader�Ŏg��
	rootParameters[3].Descriptor.ShaderRegister = 1; // ���W�X�^�ԍ��P���g��

	descriptionRootSignature.pParameters = rootParameters; // ���[�g�p�����[�^�[�z��ւ̃|�C���^
	descriptionRootSignature.NumParameters = _countof(rootParameters); // �z��̒���

	// Sampler�̐ݒ�
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // �{���j�A�t�B���^�[
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0~1�͈̔͊O�����s�[�g
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // ��r���Ȃ�
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // �����������Mipmap���g��
	staticSamplers[0].ShaderRegister = 0; // ���W�X�^�ԍ��O���g��
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShader�Ŏg��
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// �V���A���C�Y�u���ăo�C�i���ɂ���
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*> (errorBlob->GetBufferPointer()));
		assert(false);
	}
	// �o�C�i�������ɐ���
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
	hr = device->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendState�̐ݒ�
	D3D12_BLEND_DESC blendDesc{};
	// �S�Ă̐F�v�f����������
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasiterzerState�̐ݒ�
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// ���ʁi���v���j��\�����Ȃ�
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// �O�p�`�̒���h��Ԃ�
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shader���R���p�C������
	IDxcBlob* vertexShaderBlob = CompileShader(L"Resource/shaders/Object3D.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler, logStream);
	assert(vertexShaderBlob != nullptr);

	IDxcBlob* pixelShaderBlob = CompileShader(L"Resource/shaders/Object3D.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler, logStream);
	assert(pixelShaderBlob != nullptr);

	//PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get(); // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc; // InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() }; // VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() }; // PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc; // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState

	// DepthStencilState�̐ݒ�
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// Depth�̋@�\��L��������
	depthStencilDesc.DepthEnable = true;
	// �������݂��܂�
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// ��r�֐���LessEqual�B�܂�A�߂���Ε`�悳���
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// DepthStencil�̐ݒ�
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// ��������RTV�̏��
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// ���p����g�|���W�i�`��j�̃^�C�v�B�O�p�`
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// �ǂ̂悤�ɉ�ʂɐF��ł����ނ��̐ݒ�i�C�ɂ��Ȃ��ėǂ��j
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// ���ۂɐ���
	Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	//* ���f�� *//

	// �C���f�b�N�X
	Microsoft::WRL::ComPtr<ID3D12Resource> indexVertexResource = CreateBufferResource(device, sizeof(uint32_t) * 2400);

	D3D12_INDEX_BUFFER_VIEW indexBufferViewVertex{};
	// ���\�[�X�̐擪�̃A�h���X����g��
	indexBufferViewVertex.BufferLocation = indexVertexResource->GetGPUVirtualAddress();
	// �g�p���郊�\�[�X�̃T�C�Y�̓C���f�b�N�X�U���̃T�C�Y
	indexBufferViewVertex.SizeInBytes = sizeof(uint32_t) * 2400;
	// �C���f�b�N�X��uint32_t�Ƃ���
	indexBufferViewVertex.Format = DXGI_FORMAT_R32_UINT;

	// �C���f�b�N�X���\�[�X�Ƀf�[�^����������
	uint32_t* indexDataVertex = nullptr;
	indexVertexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexDataVertex));

	// ���̃C���f�b�N�X�f�[�^��ݒ肷��
	for (uint32_t i = 0; i < 2400; i += 6) {
		indexDataVertex[i + 0] = i + 0;
		indexDataVertex[i + 1] = i + 1;
		indexDataVertex[i + 2] = i + 2;
		indexDataVertex[i + 3] = i + 2;
		indexDataVertex[i + 4] = i + 1;
		indexDataVertex[i + 5] = i + 3;
	}

	// ���f���ǂݍ���
	ModelData modelData = LoadObjFile("Resource", "plane.obj");
	// ���_�o�b�t�@�p���\�[�X���쐬
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = CreateBufferResource(device, sizeof(VertexData) * modelData.vertices.size());

	// ���_�o�b�t�@�r���[���쐬����
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// �g�p���郊�\�[�X�̃T�C�Y
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	// 1���_������̃T�C�Y
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// ���_���\�[�X�Ƀf�[�^����������
	VertexData* vertexData = nullptr;
	// �������ނ��߂̃A�h���X���擾
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	// depthStencil���\�[�X
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = CreatDepthStenCilTextureResource(device, windowAPI->kClientWidth, windowAPI->kClientHeight);
	// DSV�p�̃q�[�v�Ńf�B�X�N���v�^�̐��͂P�BDSV��Shader���ŐG����̂ł͂Ȃ��̂ŁAShaderVicible��false
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	// DSV�̐ݒ�
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Format�B��{�I�ɂ�Resource�ɍ��킹��
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2dTexture
	// DSSVHeap�̐擪��DSV������
	device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// �}�e���A���p�̃��\�[�X�����B�����color�P���̃T�C�Y��p�ӂ���
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = CreateBufferResource(device, sizeof(Material));
	// �}�e���A���Ƀf�[�^����������
	Material* materialData = nullptr;
	// �������ނ��߂̃A�h���X���擾
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	// �F��ݒ肷��
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	// Lighting���邩�ǂ���
	materialData->enableLighting = true;
	// UVTransform�s��
	materialData->uvTransform = MakeIdentity4x4();


	// WVP�p�̃��\�[�X�����BMatrix4x4 1���̃T�C�Y��p�ӂ���
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = CreateBufferResource(device, sizeof(Matrix4x4));
	// �f�[�^����������
	Matrix4x4* wvpData = nullptr;
	// �������ނ��߂̃A�h���X���擾
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	// �P�ʍs�����������ł���
	*wvpData = MakeIdentity4x4();

	Microsoft::WRL::ComPtr<ID3D12Resource> transResource = CreateBufferResource(device, sizeof(TransformationMatrix));
	TransformationMatrix* transData = nullptr;
	transResource->Map(0, nullptr, reinterpret_cast<void**>(&transData));

	//*���s����*//

	// ���s�����p���\�[�X�i�萔�o�b�t�@�j���쐬
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = CreateBufferResource(device, sizeof(DirectionalLight));
	DirectionalLight* directionalLightData = nullptr;
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;
	directionalLightResource->Unmap(0, nullptr);


	//*�@�C���f�b�N�X�@*//
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = CreateBufferResource(device, sizeof(uint32_t) * 6);

	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	// ���\�[�X�̐擪�̃A�h���X����g��
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	// �g�p���郊�\�[�X�̃T�C�Y��index�U���̃T�C�Y
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	// �C���f�b�N�X��uint32_t�Ƃ���
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

	// �C���f�b�N�X���\�[�X�Ƀf�[�^����������
	uint32_t* indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0;
	indexDataSprite[1] = 1;
	indexDataSprite[2] = 2;
	indexDataSprite[3] = 1;
	indexDataSprite[4] = 3;
	indexDataSprite[5] = 2;


	//*Sprite*//

	// Sprite�p�̒��_���\�[�X�����
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = CreateBufferResource(device, sizeof(VertexData) * 6);

	// ���_�o�b�t�@�r���[���쐬
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	// ���\�[�X�̐擪�̃A�h���X����g��
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	// �g�p���郊�\�[�X�̃T�C�Y�͒��_�U�̕��̃T�C�Y
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	// 1���_������̃T�C�Y
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	// ���_�f�[�^��ݒ肷��
	VertexData* vertexDataSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	// �P���ڂ̎O�p�`
	vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };// ����
	vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	vertexDataSprite[0].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };// ����
	vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	vertexDataSprite[1].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };// �E��
	vertexDataSprite[2].texcoord = { 1.0f,1.0f };
	vertexDataSprite[2].normal = { 0.0f,0.0f,-1.0f };
	// 2���ڂ̎O�p�`
	vertexDataSprite[3].position = { 640.0f,0.0f,0.0f,1.0f };// �E��
	vertexDataSprite[3].texcoord = { 1.0f,0.0f };
	vertexDataSprite[3].normal = { 0.0f,0.0f,-1.0f };

	// Sprite�p��TransformationMatrix�p�̃��\�[�X�����BMatrix4x4 �P���̃T�C�Y��p�ӂ���
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = CreateBufferResource(device, sizeof(Matrix4x4));
	// �f�[�^����������
	Matrix4x4* transformationMatrixDataSprite = nullptr;
	// �������ނ��߂̃A�h���X���擾
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	// �P�ʍs�����������ł���
	*transformationMatrixDataSprite = MakeIdentity4x4();

	// �}�e���A���p�̃��\�[�X�����B�����color�P���̃T�C�Y��p�ӂ���
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = CreateBufferResource(device, sizeof(Material));
	// �}�e���A���Ƀf�[�^����������
	Material* materialDataSprite = nullptr;
	// �������ނ��߂̃A�h���X���擾
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));

	// �F��ݒ肷��BSprite�͔��F�ŕ\������
	materialDataSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	// Sprite��Lighting���Ȃ��̂�false��ݒ肷��
	materialDataSprite->enableLighting = false;
	// UVTransform�s��
	materialDataSprite->uvTransform = MakeIdentity4x4();

	// DescriptorSize���擾���Ă���
	const uint32_t desriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	// Texture��textureResource �ǂ�œ]��
	DirectX::ScratchImage mipImages = LoadTexture("Resource/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = CreateTextureResource(device, metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = UploadTextureData(textureResource, mipImages, device, commandList);

	// metaData�����SRV�̐ݒ�
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2D�e�N�X�`��
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	// SRV���쐬����DescriptorHeap�̏ꏊ�����߂�
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	// �擪��ImGui���g���Ă���̂ł��̎����g��
	textureSrvHandleCPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// SRV�̐���
	device->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);



	//*2���ڂ̃e�N�X�`��*//

	// 2���ڂ�Texture��ǂ�œ]������
	DirectX::ScratchImage mipImages2 = LoadTexture(modelData.material.textureFilePath);
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = CreateTextureResource(device, metadata2);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource2 = UploadTextureData(textureResource2, mipImages2, device, commandList);

	// 2����meataData�����SRV�̐ݒ�
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;// 2D�e�N�X�`��
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	// SRV���쐬����DescriptorHeap�̏ꏊ�����߂�
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = GetCPUDescriptorHandle(srvDescriptorHeap, desriptorSizeSRV, 2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = GetGPUDescriptorHandle(srvDescriptorHeap, desriptorSizeSRV, 2);
	// SRV�̐���
	device->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);

	// �r���[�|�[�g
	D3D12_VIEWPORT viewport{};
	// �N���C�A���g�̈�̃T�C�Y�ƈꏏ�ɂ��ĉ�ʑS�̂ɕ\��
	viewport.Width = windowAPI->kClientWidth;
	viewport.Height = windowAPI->kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// �V�U�[��`
	D3D12_RECT scissorRect{};
	// ��{�I�Ƀr���[�|�[�g�Ɠ�����`���\�������悤�ɂ���
	scissorRect.left = 0;
	scissorRect.right = windowAPI->kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = windowAPI->kClientHeight;


	// ImGui�̏�����
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(windowAPI->GetHwnd());
	ImGui_ImplDX12_Init(device.Get(),
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvDescriptorHeap.Get(),
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	// �f�o�b�N�J����
	DebugCamera* debugCamera = new DebugCamera();
	// �J�����̏�����
	debugCamera->Initialize();


	// �����ǂݍ���
	SoundData soundData1 = SoundLoadWave("Resource/Alarm01.wav");
	// �����Đ�
	SoundPlayWave(xAudio2, soundData1);

	MSG msg{};
	// �E�B���h�E�̂��{�^�����������܂Ń��[�v
	while (msg.message != WM_QUIT) {
		// Windows�̃��b�Z�ێ�����
		if (windowAPI->ProcessMessage()) {
			// �Q�[�����[�v�𔲂���
			break;
		}
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		// �Q�[���̏���

		// ���͂̍X�V
		input->Update();

		// �f�o�b�N�J����
		debugCamera->Update(windowAPI->GetHwnd());

		// �����̂O�L�[��������Ă�����
		if (input->TriggerKey(DIK_0)) {
			OutputDebugStringA("Hit 0\n"); // �o�̓E�B���h�E�ɁuHit �O�v�ƕ\��
		}

		// y����]����
		transform.rotate.y = 3.00f;

		Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		viewMatrix = debugCamera->GetViewMatrix(); // �f�o�b�O�J�����̃r���[�s����擾
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(windowAPI->kClientWidth) / float(windowAPI->kClientHeight), 0.1f, 100.0f);
		// WVPmatrix�����
		Matrix4x4 worldViewProjectionMatrix = Multiply(Multiply(worldMatrix, viewMatrix), projectionMatrix);
		*wvpData = worldViewProjectionMatrix;
		transData->WVP = worldViewProjectionMatrix;   // WVP�s���ݒ�
		transData->World = worldMatrix; // World�s���ݒ�



		// Sprite�p��WorldViewProjectionMatrix�����
		Matrix4x4 worldMatrixSprite = MakeAffineMatrix(tranaformSprite.scale, tranaformSprite.rotate, tranaformSprite.translate);
		Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
		Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(windowAPI->kClientWidth), float(windowAPI->kClientHeight), 0.0f, 100.0f);
		Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
		*transformationMatrixDataSprite = worldViewProjectionMatrixSprite;


		// ���ꂩ�珑�����ރo�b�N�o�b�t�@�̃C���f�b�N�X���擾

		// �X���C�_�[
		//UI
		ImGui::SliderFloat("SpritePosX", &tranaformSprite.translate.x, 0.0f, 500.0f);
		ImGui::SliderFloat("SpritePosY", &tranaformSprite.translate.y, 0.0f, 500.0f);

		// ���C�g�̌���
		ImGui::SliderFloat("directionX", &directionalLightData->direction.x, -10.0f, 10.0f);
		ImGui::SliderFloat("directionY", &directionalLightData->direction.y, -10.0f, 10.0f);
		ImGui::SliderFloat("directionZ", &directionalLightData->direction.z, -10.0f, 10.0f);

		// SRV�̐؂�ւ�
		ImGui::Checkbox("UseMonsterBall", &useMonsterBall);

		// UV���W
		ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);

		// ���f��
		ImGui::DragFloat3("scale", &transform.scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("translate", &transform.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("rotate", &transform.rotate.x, 0.01f, -10.0f, 10.0f);



		// UVTransform�p�̍s��𐶐�
		Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
		materialDataSprite->uvTransform = uvTransformMatrix;


		//Draw
		UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

		// TransitionBarrier�̐ݒ�
		D3D12_RESOURCE_BARRIER barrier{};
		// ����̃o���A��Transition
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		// None�ɂ��Ă���
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		// �o���A�𒣂�Ώۂ̃��\�[�X�B���݂̃o�b�t�@�ɑ΂��čs��
		barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();
		// �J�ڑO�i���݁j��ResourceState
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		// �J�ڌ��ResourceStare
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		// TransitionBarrier�𒣂�
		commandList->ResourceBarrier(1, &barrier);

		// �`����RTV��DSV��ݒ肷��
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);

		// �w�肵���F�ŉ�ʑS�̂��N���A����
		float clearColor[] = { 0.1f,0.25f,0.5f,1.0f }; // ���ۂ��F�BRGBA�̏�
		commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
		// �w�肵���[�x�ŉ�ʑS�̂��N���A����
		commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		// �`��p��DescriptorHeap�̐ݒ�
		ID3D12DescriptorHeap* heaps[] = { srvDescriptorHeap.Get() };
		commandList->SetDescriptorHeaps(1, heaps);


		// ����`��
		commandList->RSSetViewports(1, &viewport); // Viewport��ݒ�
		commandList->RSSetScissorRects(1, &scissorRect); // Scirssor��ݒ�
		// RootSignature��ݒ�BPSO�ɐݒ肵�Ă��邯�Ǖʓr�ݒ肪�K�v
		commandList->SetGraphicsRootSignature(rootSignature.Get());
		commandList->SetPipelineState(graphicsPipelineState.Get()); // PSO��ݒ�
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // VBV��ݒ�
		// �`���ݒ�BPSO�ɐݒ肵�Ă�����̂Ƃ͂܂��ʁB�������̂�ݒ肷��ƍl���Ă����ǂ�
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// �}�e���A��CBuffer�̏ꏊ��ݒ�
		commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		// wvp�p��World�p��CBuffer�̏ꏊ��ݒ�
		commandList->SetGraphicsRootConstantBufferView(1, transResource->GetGPUVirtualAddress());
		// SRV��DescriptorTable�̐擪��ݒ�B2��rootParameter[2]�ł���B
		commandList->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
		// ���s����
		commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

		// �C���f�b�N�X�o�b�t�@�r���[��ݒ�
		commandList->IASetIndexBuffer(&indexBufferViewVertex);
		// �C���f�b�N�X���g���ĕ`��i���j
		commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

		// Sprite�̕`��B�ύX���K�v�Ȃ��̂����ύX����
		commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);// VBV��ݒ�
		// TransformationMatrixCBuffer�̏ꏊ��ݒ�
		commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
		// SRV��DescriptorTable�̐擪��ݒ�B2��rootParameter[2]�ł���B
		commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
		// �}�e���A��CBuffer�̏ꏊ��ݒ�
		commandList->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());

		// �C���f�b�N�X�o�b�t�@�r���[��ݒ�
		commandList->IASetIndexBuffer(&indexBufferViewSprite);
		// �C���f�b�N�X���g���ĕ`��iSprite�j
		commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);


		// ���ۂ�commandList��ImGui�̕`��R�}���h���l��
		ImGui::Render();
		if (ImDrawData* draw_data = ImGui::GetDrawData()) {
			ImGui_ImplDX12_RenderDrawData(draw_data, commandList.Get());
		}

		// ��ʂɏ��������͑S�ďI���A��ʂɉf���̂ŁA��Ԃ�J��
		// �����RenderTraget����Present�ɂ���
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		// TranstionBarrier�𒣂�
		commandList->ResourceBarrier(1, &barrier);

		// �R�}���h���X�g�̓��e���m�肳����B���ׂẴR�}���h���l��ł���Close���邱��
		hr = commandList->Close();
		assert(SUCCEEDED(hr));

		// GPU�ɃR�}���h���X�g�̎��s���s�킹��
		ID3D12CommandList* commandLists[] = { commandList.Get() };
		commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

		// GPU��OS�ɉ�ʂ̌������s���悤�ʒm����
		swapChain->Present(1, 0);

		// Fence�̒l���X�V
		fenceValue++;
		// GPU�������܂ł��ǂ蒅�����Ƃ��ɁAFence�̒l���w�肵���l�ɑ��������悤��Signal�𑗂�
		commandQueue->Signal(fence.Get(), fenceValue);

		// Fence�̒l���w�肵��SIgnal�l�ɂ��ǂ蒅���Ă��邩�����ɂ񂷂�
		// GetCompletedValue�̏����l��Fence�쐬���ɓn���������l
		if (fence->GetCompletedValue() < fenceValue) {
			// �w�肵��Signal�ɂ��ǂ蒅���Ă��Ȃ��̂ŁA���ǂ蒅���܂ő҂悤�ɃC�x���g��ݒ肷��
			fence->SetEventOnCompletion(fenceValue, fenceEvent);
			// �C�x���g�҂�
			WaitForSingleObject(fenceEvent, INFINITE);
		}

		hr = commandAllocator->Reset();
		assert(SUCCEEDED(hr));

		// ���̃t���[���p�̃R�}���h���X�g������
		hr = commandList->Reset(commandAllocator.Get(), nullptr);
		assert(SUCCEEDED(hr));


	}

	// ImGui�̏I�������B�ڍׂ͂����ďd�v�ł͂Ȃ��̂ŉ���͏ȗ�����B
	//�@������������ł���B�������Ƌt���ɍs��
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// ���
	CloseHandle(fenceEvent);

	// �����f�[�^���
	xAudio2.Reset();

	// ���͂̏�����
	delete input;
	// WindowAPI�̏I������
	windowAPI->Finalize();
	// WindowAPI�̉��
	delete windowAPI;


	CoUninitialize();

	return 0;

}
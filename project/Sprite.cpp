#include "Sprite.h"
#include "SpriteCommon.h"
#include "TextureManager.h"


void Sprite::Initialize(SpriteCommon* spriteCommon,WindowAPI* windowAPI,DirectXCommon* dxCommon, std::string textureFilePath) {
	// 引数で受け取ってメンバ変数に記録する
	spriteCommon_ = spriteCommon;
	windowAPI_ = windowAPI;
	dxCommon_ = dxCommon;

	// *リソース* //
	
	// 頂点データ
	vertexResource = dxCommon_->CreateBufferResource(sizeof(VertexData) * 6);
	// インデックス
	indexResource = dxCommon_->CreateBufferResource(sizeof(uint32_t) * 6);
	// マテリアル
	materialResource = dxCommon_->CreateBufferResource(sizeof(Material));
	// 座標変換行列
	transformationMatrixResource = dxCommon_->CreateBufferResource(sizeof(TransformationMatrix));


	// *バッファビュー* //
	
	// 頂点データ
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	// インデックス
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// *データを書き込む* //

	// 頂点データ
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// １枚目の三角形
	vertexData[0].position = { 0.0f,1.0f,0.0f,1.0f };// 左下
	vertexData[0].texcoord = { 0.0f,1.0f };
	vertexData[0].normal = { 0.0f,0.0f,-1.0f };

	vertexData[1].position = { 0.0f,0.0f,0.0f,1.0f };// 左上
	vertexData[1].texcoord = { 0.0f,0.0f };
	vertexData[1].normal = { 0.0f,0.0f,-1.0f };

	vertexData[2].position = { 1.0f,1.0f,0.0f,1.0f };// 右下
	vertexData[2].texcoord = { 1.0f,1.0f };
	vertexData[2].normal = { 0.0f,0.0f,-1.0f };
	// 2枚目の三角形
	vertexData[3].position = { 1.0f,0.0f,0.0f,1.0f };// 右上
	vertexData[3].texcoord = { 1.0f,0.0f };
	vertexData[3].normal = { 0.0f,0.0f,-1.0f };
	// インデックス
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 1;
	indexData[4] = 3;
	indexData[5] = 2;
	// マテリアル
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();
	// 座標変換行列
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));

	// *テクスチャ* //

	TextureManager::GetInstance()->LoadTexture(textureFilePath);
	textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

	// TextureをtextureResource 読んで転送
	//DirectX::ScratchImage mipImages = dxCommon_->LoadTexture("Resource/uvChecker.png");
	//const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	//textureResource = dxCommon_->CreateTextureResource(metadata);
	//intermediateResource = dxCommon_->UploadTextureData(textureResource, mipImages);
	//
	//// metaDataを基にSRVの設定
	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	//srvDesc.Format = metadata.format;
	//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	//srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	//
	//// SRVを作成するDescriptorHeapの場所を決める
	//textureSrvHandleCPU = dxCommon_->srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//textureSrvHandleGPU = dxCommon_->srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	//// 先頭はImGuiを使っているのでその次を使う
	//textureSrvHandleCPU.ptr += dxCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//textureSrvHandleGPU.ptr += dxCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//// SRVの生成
	//dxCommon_->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);

}

// 更新
void Sprite::Update() {
	// 座標
	transform.translate = { position.x,position.y,0.0f };
	// 回転
	transform.rotate = { 0.0f,0.0f,rotation };
	// サイズ
	transform.scale = { size.x,size.y,1.0f };

	// アンカーポイント
	float left = 0.0f - anchorPoint.x;
	float right = 1.0f - anchorPoint.x;
	float top = 0.0f - anchorPoint.y;
	float bottom = 1.0f - anchorPoint.y;

	// 左右反転
	if (isFlipX_) {
		left = -left;
		right = -right;
	}
	// 上下反転
	if (isFlipY_) {
		top = -top;
		bottom = -bottom;
	}

	// 頂点データ更新
	vertexData[0].position = { left,bottom,0.0f,1.0f };// 左下
	vertexData[1].position = { left,top,0.0f,1.0f };// 左上
	vertexData[2].position = { right,bottom,0.0f,1.0f };// 右下
	vertexData[3].position = { right,top,0.0f,1.0f };// 右上


	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(windowAPI_->kClientWidth), float(windowAPI_->kClientHeight), 0.0f, 100.0f);
	// WVPmatrixを作る
	Matrix4x4 worldViewProjectionMatrix = Multiply(Multiply(worldMatrix, viewMatrix), projectionMatrix);
	transformationMatrixData->WVP = worldViewProjectionMatrix;   // WVP行列を設定
	transformationMatrixData->World = worldMatrix; // World行列を設定
}

void Sprite::Draw() {
	// *設定* //

	// 頂点データ
	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);// VBVを設定
	// インデックス
	dxCommon_->GetCommandList()->IASetIndexBuffer(&indexBufferView);
	
	// *場所を設定* //

	// マテリアル
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// 座標変換行列
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	
	// SRVのDescriptorTableの先頭を設定
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex));
	// インデックスを使って描画
	dxCommon_->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

}

// テクスチャ変更
void Sprite::ChangeTexture(const std::string& textureFilePath) {
	TextureManager::GetInstance()->LoadTexture(textureFilePath);

	// indexを差し替える
	textureIndex =
		TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);
}

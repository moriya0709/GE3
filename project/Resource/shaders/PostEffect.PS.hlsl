Texture2D<float4> gCurrentTexture : register(t0);
Texture2D<float4> gPreviousTexture : register(t1);
Texture2D<float> gDepthTexture : register(t2);

SamplerState gSampler : register(s0);

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

struct EffectData
{
    int isInversion;
    int isGrayscale;
    int isRadialBlur;
    int isDistanceFog;

    int isHeightFog;
    float intensity;
    float2 pad0; // float2 = 8byte

    float2 blurCenter;
    float blurWidth;
    int blurSamples;

    float3 distanceFogColor;
    float distanceFogStart;

    float distanceFogEnd;
    float zNear;
    float zFar;
    float pad1;

    float3 heightFogColor;
    float heightFogTop;

    float heightFogBottom;
    float heightFogDensity;
    float2 pad2; // ★ここが行列の直前

    float4x4 matInverseViewProjection;

    float4 finalPad[7]; // 全体を256の倍数にする
};
ConstantBuffer<EffectData> gEffectData : register(b0);

float4 main(PSInput input) : SV_TARGET
{
    float4 color = gCurrentTexture.Sample(gSampler, input.uv);

    // モノクロ
    if (gEffectData.isGrayscale)
    {
        float gray = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));
        color.rgb = float3(gray, gray, gray);
    }

    // 色反転
    if (gEffectData.isInversion)
    {
        color.rgb = 1.0f - color.rgb;
    }
    
    // 放射状ブラー
    if (gEffectData.isRadialBlur)
    {
        float2 direction = input.uv - gEffectData.blurCenter;
        float4 blurColor = color;

        for (int i = 1; i < gEffectData.blurSamples; i++)
        {
            // 中心に向かってサンプリング点をずらしていく
            float2 offset = direction * gEffectData.blurWidth * float(i);
            blurColor += gCurrentTexture.Sample(gSampler, input.uv - offset);
        }
        
        // 合計をサンプル数で割って平均化
        color = blurColor / float(gEffectData.blurSamples);
    } 
    // ディスタンスフォグ
    if (gEffectData.isDistanceFog)
    {
        // 1. 深度値の取得 (通常 0.0 ～ 1.0 の非線形な値)
        float depth = gDepthTexture.Sample(gSampler, input.uv);

        // 2. 深度値の線形化 (実際のカメラからの距離に変換)
        float linearDepth = (gEffectData.zNear * gEffectData.zFar) / (gEffectData.zFar - depth * (gEffectData.zFar - gEffectData.zNear));

        // 3. フォグ係数の計算 (saturateで 0.0 ～ 1.0 に収める)
        float fogFactor = saturate((linearDepth - gEffectData.distanceFogStart) / (gEffectData.distanceFogEnd - gEffectData.distanceFogStart));

        // 4. 元の色とフォグ色を合成
        color.rgb = lerp(color.rgb, gEffectData.distanceFogColor, fogFactor);
    }
    // ハイトフォグ
    if (gEffectData.isHeightFog)
    {
        // 1. 深度値を取得
        float depth = gDepthTexture.Sample(gSampler, input.uv);

        // 2. 画面空間(UV + Depth)からワールド座標を復元
        // NDC座標を作成 (x: -1~1, y: -1~1, z: 0~1)
        float2 ndcXY = input.uv * 2.0f - 1.0f;
        ndcXY.y *= -1.0f; // UVのYは下がプラスなので反転
        float4 ndcPos = float4(ndcXY, depth, 1.0f);

        // 逆行列を掛けてワールド座標へ
        float4 worldPosWithW = mul(gEffectData.matInverseViewProjection, ndcPos);
        float3 worldPos = worldPosWithW.xyz / worldPosWithW.w;

        // 3. 高さに基づいたフォグ係数の計算
        // 指定したTop～Bottomの間で線形補間
        float heightFactor = saturate((gEffectData.heightFogTop - worldPos.y) / (gEffectData.heightFogTop - gEffectData.heightFogBottom));
        
        // 密度(Density)を適用して濃さを調整
        heightFactor = pow(heightFactor, gEffectData.heightFogDensity);

        // 4. 合成 
        color.rgb = lerp(color.rgb, gEffectData.heightFogColor, heightFactor);
    }
    
    color.rgb *= gEffectData.intensity;
    return color;
}

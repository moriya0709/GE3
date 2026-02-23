Texture2D<float4> gCurrentTexture : register(t0);
Texture2D<float4> gPreviousTexture : register(t1);
SamplerState gSampler : register(s0);

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

struct EffectData
{
    int isInversion; // 色反転
    int isGrayscale; // モノクロ
    int isRadialBlur; // 放射状ブラー
    float intensity; // 全体の強さ
    float2 blurCenter; // ブラーの中心 (通常は 0.5, 0.5)
    float blurWidth; // ブラーの強さ (0.01～0.1程度)
    int blurSamples; // サンプリング数 (10～20程度)
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
    
    color.rgb *= gEffectData.intensity;
    return color;
}

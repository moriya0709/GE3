Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

struct EffectData
{
    int isInversion; // 1なら色反転
    int isGrayscale; // 1ならモノクロ
    float intensity; // 全体の強さ
    float pad; // 16バイトアライメント調整用
};
ConstantBuffer<EffectData> gEffectData : register(b0);

float4 main(PSInput input) : SV_TARGET
{
    float4 color = gTexture.Sample(gSampler, input.uv);

    // エフェクト1: モノクロ (適用後にcolorが上書きされる)
    if (gEffectData.isGrayscale)
    {
        float gray = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));
        color.rgb = float3(gray, gray, gray);
    }

    // エフェクト2: 色反転 (モノクロ化した色に対してさらに反転がかかる)
    if (gEffectData.isInversion)
    {
        color.rgb = 1.0f - color.rgb;
    }

    color.rgb *= gEffectData.intensity; // 最後に全体の強度を調整
    return color;
}

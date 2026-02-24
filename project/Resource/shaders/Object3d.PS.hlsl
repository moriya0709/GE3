#include "object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
};
    
struct DirectionalLight
{
    float32_t4 color; // ライトの色 
    float32_t3 direction; // ライトの向き 
    float intensity; // 輝度 
    int isDisplay; // 表示するかどうか 
    float padding[3]; // バイト合わせ 
};

struct AmbientLight
{
    float32_t4 color; // ライトの色 
    float intensity; // 輝度 
    int isDisplay; // 表示するかどうか 
    float padding[2]; // バイト合わせ 
};

struct PointLight
{
    float32_t4 color;
    float32_t3 position;
    float intensity;
    float radius;
    int isDisplay;
    float padding[2]; // バイト合わせ 
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b2);
ConstantBuffer<AmbientLight> gAmbientLight : register(b3);
ConstantBuffer<PointLight> gPointLight : register(b4);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
        
    if (gMaterial.enableLighting != 0)
    {
        float3 normal = normalize(input.normal);
        float4 directional = float4(0, 0, 0, 0);
        float4 ambient = float4(0, 0, 0, 0);
        float4 pointLight = float4(0, 0, 0, 0);
       
        // 平行光
        if (gDirectionalLight.isDisplay)
        {
            float NdotL = dot(normal, -gDirectionalLight.direction);
            float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
            directional = gDirectionalLight.color * cos * gDirectionalLight.intensity;
        }
        // 環境光 
        if (gAmbientLight.isDisplay)
        {
            ambient = gAmbientLight.color * gAmbientLight.intensity;
        }
        // ポイントライト
        if (gPointLight.isDisplay)
        {
            float3 lightVec = gPointLight.position - input.worldPosition;
            float distance = length(lightVec);
            float3 L = normalize(lightVec);

            float attenuation = saturate(1.0f - distance / gPointLight.radius);

            float NdotL = dot(normal, L);
            float diffuse = saturate(NdotL);

            pointLight = gPointLight.color * diffuse * attenuation * gPointLight.intensity;
        }
        
        float4 lighting = directional + ambient + pointLight;
        output.color = gMaterial.color * textureColor * lighting;
    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }
    return output;
}
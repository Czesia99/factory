struct VSInput {
    [[vk::location(0)]] float3 inPosition : POSITION;
    [[vk::location(1)]] float3 inNormal : NORMAL;
    [[vk::location(2)]] float2 inTexCoord : TEXCOORD0;
    [[vk::location(3)]] float4 inTangent : TANGENT;
};

struct DirLight {
    float4 direction;
    float4 color;
    float4 ambient;
};

struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
    DirLight light;
    float3 camPos;
};

ConstantBuffer<UniformBuffer> ubo;

struct VSOutput
{
    float4 pos : SV_Position;
    float3 normal;
    float2 fragTexCoord;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;

    output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.inPosition, 1.0))));

    float3x3 normalMatrix = (float3x3)ubo.model;
    output.normal = mul(normalMatrix, input.inNormal);

    output.fragTexCoord = input.inTexCoord;
    return output;
}

Sampler2D texture;
Sampler2D texture2;
Sampler2D texture3;
Sampler2D texture4;


[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    float3 N = normalize(vertIn.normal);
    float3 L = normalize(-ubo.light.direction.xyz);

    float diff = max(dot(N, L), 0.0);

    float3 diffuseColor = ubo.light.color.rgb;
    float intensity = ubo.light.color.a;

    float3 diffuse = diff * diffuseColor * intensity;
    float3 ambient = ubo.light.ambient.rgb;

    float3 totalLighting = saturate(ambient + diffuse);

    float4 texColor = texture.Sample(vertIn.fragTexCoord);
    return float4(texColor.rgb * totalLighting, texColor.a);
}

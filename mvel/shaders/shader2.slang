static const float PI = 3.14159265359;

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
    float4x4 view;
    float4x4 proj;
    DirLight light;
    float3 camPos;
};

struct ObjBufferObject {
    float4x4 model;
};

struct PushConstants {
    uint objectIndex;
};
[[vk::push_constant]] PushConstants push;

[[vk::binding(0, 0)]] ConstantBuffer<UniformBuffer> ubo;
[[vk::binding(1, 0)]] StructuredBuffer<ObjBufferObject> objectSSBO;

[[vk::binding(0, 1)]] Sampler2D textureMap;
[[vk::binding(1, 1)]] Sampler2D texture2;
[[vk::binding(2, 1)]] Sampler2D texture3;
[[vk::binding(3, 1)]] Sampler2D texture4;

struct VSOutput {
    float4 pos : SV_Position;
    float3 normal;
    float2 fragTexCoord;
    float4 tangent;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;

    float4x4 modelMatrix = objectSSBO[push.objectIndex].model;

    output.pos = mul(ubo.proj, mul(ubo.view, mul(modelMatrix, float4(input.inPosition, 1.0))));

    float3x3 normalMatrix = (float3x3)modelMatrix;
    output.normal = mul(normalMatrix, input.inNormal);
    output.tangent = input.inTangent; //float4(normalize(mul(normalMatrix, input.inTangent.xyz)), input.inTangent.w);

    output.fragTexCoord = input.inTexCoord;
    return output;
}

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

    float4 texColor = textureMap.Sample(vertIn.fragTexCoord);
    return float4(texColor.rgb * totalLighting, texColor.a);
}

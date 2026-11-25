struct CameraUniforms {
    float4x4 view;
    float4x4 proj;
};

[[vk::binding(0, 0)]] ConstantBuffer<CameraUniforms> camera;
[[vk::binding(0, 1)]] Sampler2D skyFace;

struct VSInput {
    [[vk::location(0)]] float3 position : POSITION;
    [[vk::location(2)]] float2 uv : TEXCOORD0;
};

struct VSOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    float3 viewPos = mul((float3x3)camera.view, input.position);
    float4 clip = mul(camera.proj, float4(viewPos, 1.0));
    output.position = clip.xyww;
    output.uv = input.uv;
    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_TARGET {
    uint width, height;
    skyFace.GetDimensions(width, height);

    float2 halfTexel = 0.5 / float2(width, height);
    float2 uv = clamp(input.uv, halfTexel, 1.0 - halfTexel);
    return float4(skyFace.SampleLevel(uv, 0.0).rgb, 1.0);
}

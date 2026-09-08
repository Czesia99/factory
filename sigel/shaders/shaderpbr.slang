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
    float4x4 model;
    float4x4 view;
    float4x4 proj;
    DirLight light;
    float3 camPos;
};

ConstantBuffer<UniformBuffer> ubo;

struct VSOutput {
    float4 pos : SV_Position;
    float3 worldPos;
    float3 normal;
    float4 tangent;
    float2 fragTexCoord;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;

    // Calculate World Position
    float4 worldPos = mul(ubo.model, float4(input.inPosition, 1.0));
    output.worldPos = worldPos.xyz;

    // Calculate Screen Position
    output.pos = mul(ubo.proj, mul(ubo.view, worldPos));

    // Calculate Normal and Tangent in World Space
    float3x3 normalMatrix = (float3x3)ubo.model;
    output.normal = normalize(mul(normalMatrix, input.inNormal));
    output.tangent = float4(normalize(mul(normalMatrix, input.inTangent.xyz)), input.inTangent.w);

    output.fragTexCoord = input.inTexCoord;
    return output;
}

float DistributionGGX(float3 N, float3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / max(denom, 0.0000001);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


Sampler2D diffuseMap;
Sampler2D metallic;
Sampler2D roughnessMap;
Sampler2D normalMap;

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    // 1. Sample Textures (convert Albedo to Linear space)
    float4 albedoTex = diffuseMap.Sample(vertIn.fragTexCoord);

    if (albedoTex.a < 0.5) {
        discard;
    }

    float3 albedo = pow(albedoTex.rgb, float3(2.2, 2.2, 2.2));

    float metalness = metallic.Sample(vertIn.fragTexCoord).r;
    float roughness = roughnessMap.Sample(vertIn.fragTexCoord).r;

    // 2. Normal Mapping (Calculate TBN Matrix)
    float3 tangentNormal = normalMap.Sample(vertIn.fragTexCoord).xyz * 2.0 - 1.0;

    float3 N_geom = normalize(vertIn.normal);
    float3 T = normalize(vertIn.tangent.xyz);
    T = normalize(T - dot(T, N_geom) * N_geom); // Re-orthogonalize (Gram-Schmidt)
    float3 B = cross(N_geom, T) * vertIn.tangent.w;
    float3x3 TBN = float3x3(T, B, N_geom);

    float3 N = normalize(mul(tangentNormal, TBN));

    // 3. PBR Vectors
    float3 V = normalize(ubo.camPos - vertIn.worldPos);
    float3 L = normalize(-ubo.light.direction.xyz);
    float3 H = normalize(V + L); // Halfway vector

    // 4. Base Reflectivity (F0)
    // Non-metals use 0.04, metals use their albedo color
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metalness);

    // 5. Cook-Torrance BRDF Equation
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    float3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0);

    float3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    float3 specular     = numerator / denominator;

    // 6. Energy Conservation
    float3 kS = F; // Specular contribution
    float3 kD = float3(1.0, 1.0, 1.0) - kS; // Diffuse contribution
    kD *= 1.0 - metalness; // Pure metals have no diffuse light

    // 7. Outgoing Radiance
    float NdotL = max(dot(N, L), 0.0);
    float3 radiance = ubo.light.color.rgb * ubo.light.color.a;

    // Final lighting equation for this directional light
    float3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

    // 8. Ambient Lighting (Simplified)
    // (In a full engine, this is usually replaced by Image Based Lighting / Irradiance Maps)
    float3 ambientColor = max(ubo.light.ambient.rgb * ubo.light.ambient.a, float3(0.03, 0.03, 0.03));
    float3 ambient = ambientColor * albedo;
    // float3 ambient = ubo.light.ambient.rgb * albedo * ubo.light.ambient.a;

    float3 color = ambient + Lo;

    // 9. HDR Tonemapping (Reinhard) & Gamma Correction
    color = color / (color + float3(1.0, 1.0, 1.0));
    color = pow(color, float3(1.0/2.2, 1.0/2.2, 1.0/2.2));

    return float4(color, albedoTex.a);
}

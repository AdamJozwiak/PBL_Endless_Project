// /////////////////////////////////////////////////////////////////// Inputs //
struct PixelShaderInput {
    float3 positionWorld : POSITION;
    float3 normalWorld : NORMAL;
    float3 tangent : TANGENT;
    float2 texCoord : TEXCOORD;
    float4 position : SV_POSITION;
};

// ///////////////////////////////////////////////////////////////// Textures //
Texture2D textures[4];
SamplerState textureSampler;

// //////////////////////////////////////////////////////////////// Constants //
static const float PI = 3.14159265359;
static const int TEXTURE_ALBEDO = 0, TEXTURE_AMBIENT_OCCLUSION = 1,
                 TEXTURE_METALLIC_SMOOTHNESS = 2, TEXTURE_NORMAL = 3;

// ///////////////////////////////////////////////////////// Constant buffers //
cbuffer LightParameters : register(b10) {
    float3 lightPositionWorld;
    float3 viewPositionWorld;

    float3 diffuseColor;

    float attenuationConstant;
    float attenuationLinear;
    float attenuationQuadratic;
};

// /////////////////////////////////////////////////////////// Normal mapping //
float3 calculateMappedNormal(PixelShaderInput input) {
    float3 tangent =
        normalize(input.tangent -
                  dot(input.tangent, input.normalWorld) * input.normalWorld);

    return normalize(mul((2.0f * textures[TEXTURE_NORMAL]
                                     .Sample(textureSampler, input.texCoord)
                                     .xyz -
                          float3(1.0f, 1.0f, 1.0f)),
                         float3x3(tangent, cross(tangent, input.normalWorld),
                                  input.normalWorld)));
}

// /////////////////////////////////////////////// Physically Based Rendering //
float distributionGGX(float3 n, float3 h, float roughness) {
    float a = pow(roughness, 4.0f);

    return a / (PI *
                pow(pow(max(dot(n, h), 0.0f), 2.0f) * (a - 1.0f) + 1.0f, 2.0f));
}

float geometrySchlickGGX(float nDotV, float roughness) {
    float k = pow((roughness + 1.0f), 2.0f) / 8.0f;

    return nDotV / (nDotV * (1.0f - k) + k);
}

float geometrySmith(float3 n, float3 v, float3 l, float roughness) {
    return geometrySchlickGGX(max(dot(n, l), 0.0f), roughness) *
           geometrySchlickGGX(max(dot(n, v), 0.0f), roughness);
}

float3 fresnelSchlick(float cosTheta, float3 f0) {
    return f0 + (1.0f - f0) * pow(1.0f - cosTheta, 5.0f);
}

float4 pbr(PixelShaderInput input, float3 lightDir, float factor,
           float3 normal) {
    // Load texture parameters
    float3 albedo =
        pow(textures[TEXTURE_ALBEDO].Sample(textureSampler, input.texCoord).rgb,
            2.2f * float3(1.0f, 1.0f, 1.0f));
    float ao = textures[TEXTURE_AMBIENT_OCCLUSION]
                   .Sample(textureSampler, input.texCoord)
                   .r;
    float metalness = textures[TEXTURE_METALLIC_SMOOTHNESS]
                          .Sample(textureSampler, input.texCoord)
                          .r;
    float roughness = 1.0f - textures[TEXTURE_METALLIC_SMOOTHNESS]
                                 .Sample(textureSampler, input.texCoord)
                                 .a;

    // Calculate view direction
    float3 viewDir = normalize(viewPositionWorld - input.positionWorld);

    // Radiance
    float3 h = normalize(viewDir + lightDir);
    float3 radiance = diffuseColor * factor;

    // Cook-Torrance BRDF
    float ndf = distributionGGX(normal, h, roughness);
    float g = geometrySmith(normal, viewDir, lightDir, roughness);
    float3 f = fresnelSchlick(
        max(dot(h, viewDir), 0.0f),
        lerp(0.04f * float3(1.0f, 1.0f, 1.0f), albedo, metalness));

    float3 kD = (float3(1.0, 1.0f, 1.0f) - f) * (1.0f - metalness);

    float3 specular =
        (ndf * g * f) / max((4.0f * max(dot(normal, viewDir), 0.0f) *
                             max(dot(normal, lightDir), 0.0f)),
                            0.001f);

    return float4((kD * albedo / PI + specular) * radiance *
                      max(dot(normal, lightDir), 0.0f),
                  1.0f);
}

// ////////////////////////////////////////////////////////////// Light types //
float attenuate(float distance) {
    return 1.0f / (attenuationConstant + attenuationLinear * distance +
                   attenuationQuadratic * pow(distance, 2.0f));
}

float4 pointLight(PixelShaderInput input, float3 normal) {
    float3 lightDir = normalize(lightPositionWorld - input.positionWorld);
    return pbr(input, lightDir,
               attenuate(length(lightPositionWorld - input.positionWorld)),
               normal);
}

// ///////////////////////////////////////////////////////////////////// Main //
float4 main(PixelShaderInput input) : SV_Target {
    float3 normal = calculateMappedNormal(input);

    float4 outColor =
        clamp(pointLight(input, normal), float4(0.0f, 0.0f, 0.0f, 0.0f),
              float4(1.0f, 1.0f, 1.0f, 1.0f));

    float4 pixelColor = float4(textures[TEXTURE_AMBIENT_OCCLUSION]
                                       .Sample(textureSampler, input.texCoord)
                                       .rgb *
                                   outColor.rgb,
                               1.0f);

    outColor = pow(pixelColor, (1.0f / 2.2f) * float4(1.0f, 1.0f, 1.0f, 1.0f));

    return outColor;
}

// ////////////////////////////////////////////////////////////////////////// //

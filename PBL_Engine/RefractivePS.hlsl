// /////////////////////////////////////////////////////////////////// Inputs //
struct PixelShaderInput {
    float3 positionWorld : POSITION;
    float3 normalWorld : NORMAL;
    float3 tangentWorld : TANGENT;
    float3 bitangentWorld : BITANGENT;
    float2 texCoord : TEXCOORD;
    float4 position : SV_POSITION;
};

// ///////////////////////////////////////////////////////////////// Texture //
TextureCube skybox : register(t6);
SamplerState textureSampler {
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

// //////////////////////////////////////////////////////////////// Constants //
static const float refractFactor = 1.0f / 1.33f;
static const int NUM_LIGHTS = 8;

// ///////////////////////////////////////////////////////// Constant buffers //
cbuffer Transform : register(b0) {
    matrix model;
    matrix modelInverseTranspose;
    matrix viewProj;
};

cbuffer LightParameters : register(b10) {
    float4 lightPositionWorld[NUM_LIGHTS];
    float4 viewPositionWorld;

    float4 diffuseColor[NUM_LIGHTS];

    float4 intensity[NUM_LIGHTS / 4];

    float attenuationConstant;
    float attenuationLinear;
    float attenuationQuadratic;
};
// ///////////////////////////////////////////////////////////////////// Main //
float4 main(PixelShaderInput input) : SV_TARGET {
    float3 refractionDirection = normalize(input.position - viewPositionWorld);
    float3 refractionCoord = refract(
        refractionDirection, normalize(input.normalWorld), refractFactor);

    return skybox.Sample(textureSampler, refractionCoord);

}
// /////////////////////////////////////////////////////////////////// Inputs //
struct PixelShaderInput {
    float3 positionWorld : POSITION;
    float4 position : SV_POSITION;
};

// ////////////////////////////////////////////////////////////////// Outputs //
struct PixelShaderOutput {
    float4 color : SV_Target0;
};

// //////////////////////////////////////////////////////////////// Constants //
static const int NUM_LIGHTS = 16;

// ///////////////////////////////////////////////////////// Constant buffers //
cbuffer LightParameters : register(b10) {
    float4 lightPositionWorld[NUM_LIGHTS];
    float4 viewPositionWorld;

    float4 diffuseColor[NUM_LIGHTS];

    float4 intensity[NUM_LIGHTS / 4];
    float4 attenuationConstant[NUM_LIGHTS / 4];
    float4 attenuationLinear[NUM_LIGHTS / 4];
    float4 attenuationQuadratic[NUM_LIGHTS / 4];

    float4 mainLightPos;
};

// ///////////////////////////////////////////////////////////////////// Main //
PixelShaderOutput main(PixelShaderInput input) {
    PixelShaderOutput output;
    output.color = length(input.positionWorld - mainLightPos);

    return output;
}

// ////////////////////////////////////////////////////////////////////////// //

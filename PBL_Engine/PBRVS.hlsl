// /////////////////////////////////////////////////////////////////// Inputs //
struct VertexShaderInput {
    float3 positionModel : POSITION;
    float3 normalModel : NORMAL;
    float3 tangentModel : TANGENT;
    float3 bitangentModel : BITANGENT;
    float2 texCoord : TEXCOORD;
#ifdef ANIMATED
    uint4 boneIndices : BONEID;
    float4 boneWeights : BONEWEIGHT;
#endif
};

// ////////////////////////////////////////////////////////////////// Outputs //
struct VertexShaderOutput {
    float3 positionWorld : POSITION;
    float3 normalWorld : NORMAL;
    float3 tangentWorld : TANGENT;
    float3 bitangentWorld : BITANGENT;
    float2 texCoord : TEXCOORD;
    float4 position : SV_POSITION;
};

// //////////////////////////////////////////////////////////////// Constants //
static const int MAX_BONES = 256;

// ///////////////////////////////////////////////////////// Constant buffers //
cbuffer Transform : register(b0) {
    matrix model;
    matrix modelInverseTranspose;
    matrix viewProj;
};

cbuffer Animation : register(b1) { matrix boneTransforms[MAX_BONES]; };

// TODO: Just a test for this cool displacement effect, needs solid rework and
// adjustments but has some potential for the dream/memory-like stuff :)
// This is just a temporary copy of the structure found in the pixel shader, we
// need to find a simpler and more direct way to get access to some variables
// for the displacement effect here
static const int NUM_LIGHTS = 16;
cbuffer LightParameters : register(b11) {
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
VertexShaderOutput main(VertexShaderInput input) {
    VertexShaderOutput output;

    matrix world = model;
    matrix worldInverseTranspose = modelInverseTranspose;
    float4 positionModel = float4(input.positionModel, 1.0f);

#ifdef ANIMATED
    matrix skinning = boneTransforms[input.boneIndices.x] * input.boneWeights.x;
    skinning += boneTransforms[input.boneIndices.y] * input.boneWeights.y;
    skinning += boneTransforms[input.boneIndices.z] * input.boneWeights.z;
    skinning += boneTransforms[input.boneIndices.w] * input.boneWeights.w;

    world = mul(skinning, world);
    worldInverseTranspose = mul(skinning, worldInverseTranspose);
#endif

    output.positionWorld = mul(positionModel, world).xyz;
    output.normalWorld =
        mul(input.normalModel, (float3x3)worldInverseTranspose);
    output.tangentWorld = mul(input.tangentModel, (float3x3)world);
    output.bitangentWorld = mul(input.bitangentModel, (float3x3)world);
    output.position = mul(positionModel, mul(world, viewProj));
    output.texCoord = input.texCoord;

    // TODO: Just a test for this cool displacement effect, needs solid rework
    // and adjustments but has some potential for the dream/memory-like stuff
    // :)
    if (mainLightPos.x - 10 > output.positionWorld.x) {
        output.position.y /= (1 + 0.02 * distance(mainLightPos.x - 10, output.positionWorld.x));
    }
    if (mainLightPos.x + 30 < output.positionWorld.x) {
        output.position.y *= (1 + 0.1 * distance(mainLightPos.x + 30, output.positionWorld.x));
    }

    return output;
}

// ///////////////////////////////////////////////////////////////////////// //

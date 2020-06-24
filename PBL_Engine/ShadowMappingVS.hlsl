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

    return output;
}

// ///////////////////////////////////////////////////////////////////////// //

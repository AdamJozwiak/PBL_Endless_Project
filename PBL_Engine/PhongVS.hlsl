#define MAX_BONES 256

cbuffer TransformBuffer : register(b0) {
    matrix model;
    matrix viewProj;
};

cbuffer AnimationBuffer : register(b1) {
    matrix boneTransforms[MAX_BONES];
};

struct VertexShaderInput {
    float3 position : Position;
    float3 normal : Normal;
#ifdef ANIMATED
    uint4 boneIndices : BoneID;
    float4 boneWeights : BoneWeight;
#endif
};

struct VertexShaderOutput {
    float3 worldPos : Position;
    float3 normal : Normal;
    float4 position : SV_Position;
};

VertexShaderOutput main(VertexShaderInput input) {
    VertexShaderOutput output;
    matrix world = model;
    float4 position = float4(input.position, 1.0f);

#ifdef ANIMATED
    matrix skinning = boneTransforms[input.boneIndices.x] * input.boneWeights.x;
    skinning += boneTransforms[input.boneIndices.y] * input.boneWeights.y;
    skinning += boneTransforms[input.boneIndices.z] * input.boneWeights.z;
    skinning += boneTransforms[input.boneIndices.w] * input.boneWeights.w;

    world = mul(skinning, world);
#endif

    output.worldPos = (float3)mul(position, world);
    output.normal = mul(input.normal, (float3x3)world);
    output.position = mul(position, mul(world, viewProj));

    return output;
}

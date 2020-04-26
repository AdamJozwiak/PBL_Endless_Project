#define B_SLOT_TRANSFORM b0
#define B_SLOT_BONES b5
#define B_SLOT_0 b2
#define B_SLOT_1 b3
#define B_SLOT_2 b4
#define B_SLOT_3 b5
#define B_SLOT_4 b6
#define B_SLOT_5 b7
#define B_SLOT_6 b8

#define MAX_BONES 256

cbuffer SkeletalAnimationData : register(B_SLOT_BONES) {
    matrix BoneTransforms[MAX_BONES];
};

struct VertexInputType {
    float3 position : Position;
    float3 normal : Normal;
//#ifdef HAS_BONES
    uint4 boneids : BoneID;
    float4 boneweights : BoneWeight;
//#endif
//#ifdef MASK_ALPHA
    //float2 tex : TEXCOORD;
//#endif
};

cbuffer CBuf {
    matrix model;
    matrix viewProj;
};

struct VSOut {
    float3 worldPos : Position;
    float3 normal : Normal;
    float4 pos : SV_Position;
};

VSOut main(VertexInputType vit) {
    VSOut vso;
    matrix final_world = model;

//#ifdef HAS_BONES
    matrix bone_transform = BoneTransforms[vit.boneids.x] * vit.boneweights.x;
    bone_transform += BoneTransforms[vit.boneids.y] * vit.boneweights.y;
    bone_transform += BoneTransforms[vit.boneids.z] * vit.boneweights.z;
    bone_transform += BoneTransforms[vit.boneids.w] * vit.boneweights.w;

    final_world = mul(bone_transform, final_world);
//#endif
    vso.worldPos = (float3)mul(float4(vit.position, 1.0f), final_world);
    vso.normal = mul(vit.normal, (float3x3)final_world);
    vso.pos = mul(float4(vit.position, 1.0f), mul(final_world, viewProj));
    return vso;
}
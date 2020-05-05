cbuffer TransformCBuf {
    matrix model;
    matrix viewProj;
};

struct VSOut {
    float4 pos : SV_POSITION;
    float4 worldPos : POSITION;
    float2 tex : TEXCOORD;
};

VSOut main(float3 position : POSITION, float2 textureCoords : TEXCOORD) {
    VSOut output;
    output.pos = mul(float4(position, 1.0f), mul(model, viewProj));
    output.worldPos = mul(float4(position, 1.0f), model);
    output.tex = textureCoords;
    return output;
}
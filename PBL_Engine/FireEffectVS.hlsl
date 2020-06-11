cbuffer Transform : register(b0) {
    matrix model;
    matrix modelInverseTranspose;
    matrix viewProj;
};

struct VsIn {
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct VSOut {
    float4 pos : SV_POSITION;
    float4 worldPos : POSITION;
    float2 tex : TEXCOORD;
};

VSOut main(VsIn input) {
    VSOut output;
    output.pos = mul(input.position, mul(model, viewProj));
    output.worldPos = mul(input.position, model);
    output.tex = input.tex;
    return output;
}
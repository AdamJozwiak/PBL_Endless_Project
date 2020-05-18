struct VsIn {
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct VSOut {
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD;
};

VSOut main(VsIn input) {
    VSOut output;
    output.pos = input.position;
    output.tex = input.tex;
    return output;
}
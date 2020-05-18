Texture2D ColorTexture;
SamplerState splr;

struct VSOut {
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD;
};

float4 main(VSOut input) : SV_TARGET {
    float4 color = ColorTexture.Sample(splr, input.tex);

    return float4(color.b, color.g, color.r, color.a);
}
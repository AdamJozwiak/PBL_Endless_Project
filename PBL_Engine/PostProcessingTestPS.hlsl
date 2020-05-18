static const float3 GrayScaleIntensity = {0.299f, 0.587f, 0.114f};

Texture2D ColorTexture;
SamplerState splr;

struct VSOut {
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD;
};

float4 main(VSOut input) : SV_TARGET {
    float4 color = ColorTexture.Sample(splr, input.tex);
    float intensity = dot(color.rgb, GrayScaleIntensity);

    return color;
    //return float4(intensity.rrr, color.a);
}
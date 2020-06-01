Texture2D ColorTexture : register(t0);
Texture2D Gradient : register(t20);
SamplerState splr {
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

struct VSOut {
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD;
};

static const float BRIGHTNESS = -0.025f;
static const float CONTRAST = 0.95f;
static const float CURVES_PROPORTION = 0.8f;

float4 main(VSOut input) : SV_TARGET {
    // Loading texture
    float4 color = ColorTexture.Sample(splr, input.tex);

    // brightness and contrast correction
    color = (CONTRAST * (color - 0.5f) + 0.5f) + BRIGHTNESS;

    // gamma correction
    color = pow(color, 1.0f / 2.2f);

    // curves
    float4 curves = float4(
        Gradient.Sample(splr, float2(clamp(color.r, 0.001f, 0.999f), 0.5f)).r,
        Gradient.Sample(splr, float2(clamp(color.g, 0.001f, 0.999f), 0.5f)).g,
        Gradient.Sample(splr, float2(clamp(color.b, 0.001f, 0.999f), 0.5f)).b,
        color.a);

    return lerp(color, curves, CURVES_PROPORTION);
}
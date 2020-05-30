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

static const float Brightness = 0.0f;
static const float Contrast = 1.0f;

float4 main(VSOut input) : SV_TARGET {
    // Loading texture
    float4 color = ColorTexture.Sample(splr, input.tex);

    float4 curves = float4(
        Gradient.Sample(splr, float2(clamp(color.r, 0.01f, 0.99f), 0.5f)).r,
        Gradient.Sample(splr, float2(clamp(color.g, 0.01f, 0.99f), 0.5f)).g,
        Gradient.Sample(splr, float2(clamp(color.b, 0.01f, 0.99f), 0.5f)).b,
        color.a);
    color = curves;

    color = pow(color, 2.2f * float4(1.0f, 1.0f, 1.0f, 1.0f));
    // brightness and contrast correction
    color = (color - float4(0.5f, 0.5f, 0.5f, 0.5f)) *
                (Contrast * float4(1.0f, 1.0f, 1.0f, 1.0f)) +
            float4(0.5f, 0.5f, 0.5f, 0.5f) +
            Brightness * float4(1.0f, 1.0f, 1.0f, 1.0f);
    // curves

    // gamma correction
    color = pow(color, (1.0f / 2.2f) * float4(1.0f, 1.0f, 1.0f, 1.0f));

    return float4(color.r, color.g, color.b, color.a);
}
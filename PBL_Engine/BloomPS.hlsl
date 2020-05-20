static const float3 GrayScaleIntensity = {0.299f, 0.587f, 0.114f};

Texture2D ColorTexture[2];
SamplerState splr;

struct VSOut {
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD;
};

float4 adjustSaturation(float4 color, float saturation) {
    float grey = dot(color, GrayScaleIntensity);

    return lerp(grey, color, saturation);
}

float4 main(VSOut input) : SV_TARGET {
    // variables
    float blurDistance = 0.002f;
    float4 baseColor = ColorTexture[0].Sample(splr, input.tex);
    float4 bloomColor = ColorTexture[1].Sample(splr, input.tex);
    float intensity = dot(baseColor.rgb, GrayScaleIntensity);
    float bloomIntensity = 1.2f;
    float originalIntensity = 1.0f;
    float bloomSaturation = 1.0;
    float originalSaturation = 1.0f;

    // blur
    for (int i = 0; i < 3; i++) {
        bloomColor += ColorTexture[1].Sample(
            splr,
            float2(input.tex.x + blurDistance, input.tex.y + blurDistance));
    }
    bloomColor /= 6;

    bloomColor = adjustSaturation(bloomColor, bloomSaturation) * bloomIntensity;
    baseColor =
        adjustSaturation(baseColor, originalSaturation) * originalIntensity;

    baseColor *= (1 - saturate(bloomColor));
    return baseColor + bloomColor;
    // return float4(intensity.rrr, color.a);
}
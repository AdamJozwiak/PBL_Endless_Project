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

float normpdf(in float x, in float sigma) {
    return 0.39894 * exp(-0.5 * x * x / (sigma * sigma)) / sigma;
}

float4 gaussianBlur(float blurDistance, float2 texCoords, float2 resolution,
                    Texture2D sampledTexture, SamplerState textureSampler) {
    const int mSize = 21;
    const int kSize = (mSize - 1) / 2;
    float kernel[mSize];
    float3 final_colour = float3(0, 0, 0);

    // create the 1-D kernel
    float sigma = 7.0;
    float normalizationFactor = 0.0;
    for (int j = 0; j <= kSize; ++j) {
        kernel[kSize + j] = kernel[kSize - j] = normpdf(float(j), sigma);
    }

    // get the normalization factor (as the gaussian has been clamped)
    for (int j = 0; j < mSize; ++j) {
        normalizationFactor += kernel[j];
    }

    // read out the texels
    for (int i = -kSize; i <= kSize; ++i) {
        for (int j = -kSize; j <= kSize; ++j) {
            float2 uv = texCoords + blurDistance * (float2(float(i), float(j)) /
                                                    resolution);

            uv.x = clamp(uv.x, 0.01f, 0.99f);
            uv.y = clamp(uv.y, 0.01f, 0.99f);

            final_colour += kernel[kSize + j] * kernel[kSize + i] *
                            sampledTexture.Sample(textureSampler, uv).rgb;
        }
    }

    return float4(final_colour / (normalizationFactor * normalizationFactor),
                  1.0);
}

float4 main(VSOut input) : SV_TARGET {
    // variables
    float blurDistance = 2.0f;
    float4 baseColor = ColorTexture[0].Sample(splr, input.tex);
    float4 bloomColor = ColorTexture[1].Sample(splr, input.tex);
    float intensity = dot(baseColor.rgb, GrayScaleIntensity);
    float bloomIntensity = 0.4f;
    float originalIntensity = 1.0f;
    float bloomSaturation = 0.5f;
    float originalSaturation = 1.0f;
    float2 resolution = float2(1920.0f, 1080.0f);

    // blur
    bloomColor = gaussianBlur(blurDistance, input.tex, resolution,
                              ColorTexture[1], splr);

    bloomColor = adjustSaturation(bloomColor, bloomSaturation) * bloomIntensity;
    baseColor =
        adjustSaturation(baseColor, originalSaturation) * originalIntensity;

    baseColor *= (1 - saturate(bloomColor));
    return baseColor + bloomColor;
    // return float4(intensity.rrr, color.a);
}

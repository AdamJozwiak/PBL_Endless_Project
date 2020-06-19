#include "harmonic.hlsl"

cbuffer Cbuf { float time; }

struct VSOut {
    float4 pos : SV_POSITION;
    float4 worldPos : POSITION;
    float4 localPos : NORMAL;
    float2 tex : TEXCOORD;
};
struct PixelShaderOutput {
    float4 color : SV_Target0;
    float4 bloom : SV_Target1;
};

Texture2D shaderTextures[4];
SamplerState splr;

float4 rand(float2 uv) {
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453) / 100.0f;
}

PixelShaderOutput main(VSOut input) {
    float redShine = 3.0f;
    float greenShine = 2.0f;
    float blueShine = 1.0f;

    float2 noisePadding = float2(input.tex.x + rand(input.tex).x,
                                 input.tex.y + rand(input.tex).y);

    // Noise
    float noises[4];
    noises[0] = shaderTextures[1].Sample(
        splr,
        float2(noisePadding.x * 1.0f, noisePadding.y * 1.0f + 0.9f * time));
    noises[1] = shaderTextures[1].Sample(
        splr,
        float2(noisePadding.x * 0.5f, noisePadding.y * 0.5f + 0.3f * time));
    noises[2] = shaderTextures[1].Sample(
        splr,
        float2(noisePadding.x * 1.5f, noisePadding.y * 1.5f + 0.6f * time));
    noises[3] = shaderTextures[1].Sample(
        splr,
        float2(noisePadding.x * 0.1f, noisePadding.y * 0.2f + 0.2f * time));

    // Gradient
    float gradient = shaderTextures[2].Sample(splr, input.tex);

    input.tex.x +=
        gradient * harmonic(0.01f, 0.5f, 0.2f, 0.0f, time, input.tex.y);

    // Albedo
    float4 albedo = shaderTextures[0].Sample(
        splr,
        float2(input.tex.x + 0.4 * gradient.r *
                                 sin(time * 0.5f + input.tex.y * 2) * noises[0],
               input.tex.y + 0.5 * time +
                   0.1 * gradient.r * cos(time * 0.55f + input.tex.x * 4) *
                       noises[0]));

    // Shape
    float4 shape = shaderTextures[3].Sample(
        splr,
        float2(input.tex.x + 0.2 * gradient.r *
                                 sin(time * 2 + input.tex.y * 2) * noises[0],
               input.tex.y - 0.01f * (1.0f - gradient.r) *
                                 sin(time * 1 + input.tex.y * 1) * noises[1]));

    // Handle noise data
    if (noises[0] < 0.7f * gradient.r && shape.r > 0.25f) {
        albedo.a *= noises[0] * gradient.r;
    }
    if (noises[1] < 0.8f * gradient.r && shape.r > 0.25f) {
        albedo.a *= noises[1] * gradient.r;
    }
    if (noises[2] < 0.8f * gradient.r) {
        albedo.r += 0.5f * noises[0].r;
        albedo.g += 0.5f * noises[0].r;
        albedo.b += 0.5f * noises[0].r;
    }
    if (noises[3] < 0.2f * gradient.r && shape.r > 0.25f) {
        albedo.a *= noises[3] * gradient.r;
    }

    // Emphasize fire color with noise
    albedo.r += 0.3f * noises[0].r - 0.1f * noises[1].r;
    albedo.g += 0.3f * noises[0].r - 0.1f * noises[1].r;
    albedo.b += 0.3f * noises[0].r - 0.1f * noises[1].r;

    albedo.r *= 1.0f + gradient.r * redShine +
                gradient.r * (1.0f - noises[0]) * redShine;
    albedo.g *= 1.0f + gradient.r * greenShine +
                gradient.r * (1.0f - noises[0]) * greenShine;
    albedo.b *= 1.0f + gradient.r * blueShine +
                gradient.r * (1.0f - noises[0]) * blueShine;

    // If the pixel is less transparent than the corresponding shape pixel (fire
    // alpha is bigger than shape's alpha)
    if (albedo.a > (1.0f - shape.r)) {
        albedo.a *=
            (1.0f - shape.r);  // Make the fire transparent proportionally to
                               // the shape's alpha value
    }

    // Make the fire linearly more transparent from the bottom to the top, but
    // change the gradient range from [0.0f, 1.0f] to [0.75f, 1.0f]
    albedo.a *= (1.0f - gradient.r) / 4.0f + 0.75f;

    // Fine tune the final alpha
    if (shape.r < 0.25f) {  // If the pixel is inside the fireball
        albedo.a = 1.0f;    // Make sure it's not transparent
    } else {                // But if the pixel is outside
        albedo.a = pow(
            albedo.a,
            0.375f);  // Change the alpha curve to lessen the transparency
                      // without changing the relationships between pixel alphas
    }

    static const float BLOOM_THRESHOLD = 0.3f;
    PixelShaderOutput output;
    output.color = albedo;
    output.bloom =
        saturate((output.color - BLOOM_THRESHOLD) / (1 - BLOOM_THRESHOLD));

    return output;
}

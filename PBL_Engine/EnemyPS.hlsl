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
    input.tex.x += harmonic(0.002f, 0.2f, 0.2f, 0.0f, time, input.tex.y);
    input.tex.x -= harmonic(0.01f, 2.0f, 0.5f, 0.2f, time, input.tex.x);

    input.tex.y += harmonic(0.01f, 1.0f, 0.3f, 0.1f, time, input.tex.x);
    input.tex.y -= harmonic(0.02f, 4.0f, 0.8f, 0.4f, time, input.tex.y);

    float redShine =
        1.0f + harmonic(0.02f, 4.0f, 0.5f, 0.0f, time, input.tex.x);
    float greenShine =
        1.0f + harmonic(0.03f, 3.0f, 0.6f, 0.5f, time, input.tex.y);
    float blueShine =
        1.0f + harmonic(0.02f, 2.0f, 0.7f, 0.2f, time, input.tex.x);

    float2 noisePadding = float2(input.tex.x + rand(input.tex).x,
                                 input.tex.y + rand(input.tex).y);

    // Noise
    float noises[3];
    noises[0] = shaderTextures[1].Sample(
        splr, float2((noisePadding.x - 0.5f) * 1.5f + 0.1f * -time,
                     (noisePadding.y - 0.5f) * 1.5f + 0.4f * -time));
    noises[1] = shaderTextures[1].Sample(
        splr, float2((noisePadding.x + 0.5f) * 3.5f + 0.6f * -time,
                     (noisePadding.y + 0.5f) * 3.5f + 0.4f * -time));
    noises[2] = shaderTextures[1].Sample(
        splr, float2((noisePadding.x + 1.5f) * 2.0f - 0.3f * -time,
                     (noisePadding.y + 1.5f) * 2.0f - 0.5f * -time));

    // Albedo
    float4 albedo = shaderTextures[0].Sample(
        splr, float2(input.tex.x +
                         0.4 * sin(-time * 0.4f + input.tex.y * 2) * noises[0],
                     input.tex.y + 0.1 * -time +
                         0.1 * cos(time * 0.4f + input.tex.x * 4) * noises[0]));

    // Shape
    float4 shape =
        shaderTextures[3].Sample(splr, float2(input.tex.x, input.tex.y));

    // Handle noise data
    if (noises[0] < 0.6f) {
        albedo.a *= noises[0] + 0.3f;
    }
    if (noises[1] < 0.3f) {
        albedo.a *= noises[1] + 0.3f;
    }
    if (noises[2] < 0.5f) {
        albedo.a *= noises[2];
    }

    albedo.r *= 1.0f * redShine * (1.0f - noises[0]) * redShine;
    albedo.g *= 1.0f * greenShine * (1.0f - noises[0]) * greenShine;
    albedo.b *= 1.0f * blueShine * (1.0f - noises[0]) * blueShine;

    albedo.r += shape.r * shape.r;
    albedo.g += shape.g * shape.g;
    albedo.b += shape.b * shape.b;
    albedo.a *= shape.a;

    static const float BLOOM_THRESHOLD = 0.3f;
    PixelShaderOutput output;
    output.color = albedo;
    output.bloom =
        saturate((output.color - BLOOM_THRESHOLD) / (1 - BLOOM_THRESHOLD));

    return output;
}

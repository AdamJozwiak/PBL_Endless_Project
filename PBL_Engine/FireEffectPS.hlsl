cbuffer Cbuf { float time; }

struct VSOut {
    float4 pos : SV_POSITION;
    float4 worldPos : POSITION;
    float2 tex : TEXCOORD;
};

Texture2D shaderTextures[4];
SamplerState splr;

float4 rand(float2 uv) {
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453) / 100.0f;
}

float4 main(VSOut input) : SV_TARGET {
    float redShine = 3.0f;
    float greenShine = 5.0f;
    float blueShine = 2.0f;

    float4 albedo = shaderTextures[0].Sample(splr, input.tex);
    // float4 col = float4(albedo.rgba * (1.0f, 1.0f, 1.0f, 1.0f));

    float2 noisePadding = float2(input.tex.x + rand(input.tex).x,
                                 input.tex.y + rand(input.tex).y);

    float noise = shaderTextures[1].Sample(
        splr, float2(noisePadding.x, noisePadding.y + time / 1.5f));

    float gradient = shaderTextures[2].Sample(splr, input.tex);

    if (noise > (1.5f - gradient.r) * 0.5f && noisePadding.y < 0.75f) discard;

    albedo.r *=
        1.0f + gradient.r * redShine + gradient.r * (1.0f - noise) * redShine;
    albedo.g *= 1.0f + gradient.r * greenShine +
                gradient.r * (1.0f - noise) * greenShine;
    albedo.b *=
        1.0f + gradient.r * blueShine + gradient.r * (1.0f - noise) * blueShine;

    float4 shape = shaderTextures[3].Sample(
        splr, float2(input.tex.x + 0.2 * gradient.r *
                                       sin(time * 4 + input.tex.y * 2) * noise,
                     input.tex.y));

    if (albedo.a > shape.a + 0.5f) discard;

    return albedo;
}
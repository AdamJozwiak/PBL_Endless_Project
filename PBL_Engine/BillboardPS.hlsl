Texture2D Texture;
SamplerState splr;

struct VSOut {
    float4 pos : SV_POSITION;
    float4 worldPos : POSITION;
    float2 tex : TEXCOORD;
};

float4 main(VSOut input) : SV_Target {
    // Set diffuse color of material
    float4 diffuse;
    diffuse = pow(Texture.Sample(splr, input.tex), 2.2f);
    clip(diffuse.a - 0.25);

    return float4(diffuse);
}
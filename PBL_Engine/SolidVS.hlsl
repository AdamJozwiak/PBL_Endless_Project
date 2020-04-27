cbuffer CBuf {
    matrix model;
    matrix viewProj;
};

float4 main(float3 pos : Position) : SV_Position {
    return mul(float4(pos, 1.0f), mul(model, viewProj));
}

cbuffer LightCBuf : register(b10) {
    float3 lightPos;
    float3 viewPos;
    float3 ambient;
    float3 diffuseColor;
    float diffuseIntensity;
    float attConst;
    float attLin;
    float attQuad;
};

cbuffer ObjectCBuf : register(b0) {
    float3 materialColor;
    float specularIntensity;
    float specularPower;
};
struct Output {
    float4 color : SV_Target0;
    float4 bloom : SV_Target1;
};

Output main(float3 worldPos : Position, float3 n : Normal) {
    // fragment to light vector data
    float3 lightDir = lightPos - worldPos;
    const float distToL = length(lightDir);
    lightDir = normalize(lightDir);
    float threshold = 0.3f;

    // attenuation
    const float att =
        1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));

    // diffuse intensity
    const float3 diffuse =
        diffuseColor * diffuseIntensity * att * max(0.0f, dot(lightDir, n));

    // reflected light vector
    const float3 viewDir = normalize(viewPos - worldPos);
    const float3 r = normalize(reflect(-lightDir, n));

    // calculate specular intensity based on angle between viewing vector and
    // reflection vector, narrow with power function
    const float3 specular = att * (diffuseColor * diffuseIntensity) *
                            specularIntensity *
                            pow(max(0.0f, dot(r, viewDir)), specularPower);
    Output output;
    output.color =
        float4(saturate((diffuse + ambient + specular) * materialColor), 1.0f);
    output.bloom = saturate((output.color - threshold) / (1 - threshold));

    // final color
    return output;
}

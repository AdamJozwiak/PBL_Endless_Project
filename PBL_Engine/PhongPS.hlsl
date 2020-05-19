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

float4 main(float3 worldPos : Position, float3 n : Normal) : SV_Target {
    // fragment to light vector data
    float3 lightDir = lightPos - worldPos;
    const float distToL = length(lightDir);
    lightDir = normalize(lightDir);

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
    const float3 specular =
        att * (diffuseColor * diffuseIntensity) * specularIntensity *
        pow(max(0.0f, dot(r, viewDir)), specularPower);

    // final color
    return float4(saturate((diffuse + ambient + specular) * materialColor),
                  1.0f);
}

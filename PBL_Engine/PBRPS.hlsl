// /////////////////////////////////////////////////////////////////// Inputs //
struct PixelShaderInput {
    float3 positionWorld : POSITION;
    float3 normalWorld : NORMAL;
    float3 tangentWorld : TANGENT;
    float3 bitangentWorld : BITANGENT;
    float2 texCoord : TEXCOORD;
    float4 position : SV_POSITION;
};

// ////////////////////////////////////////////////////////////////// Outputs //
struct PixelShaderOutput {
    float4 color : SV_Target0;
    float4 bloom : SV_Target1;
};

// ///////////////////////////////////////////////////////////////// Textures //
Texture2D textures[5];
SamplerState textureSampler {
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
    AddressW = Wrap;
};

TextureCube shadowMap : register(t11);

// //////////////////////////////////////////////////////////////// Constants //
static const int NUM_LIGHTS = 16;
static const float PI = 3.14159265359;
static const int TEXTURE_ALBEDO = 0, TEXTURE_AMBIENT_OCCLUSION = 1,
                 TEXTURE_METALLIC_SMOOTHNESS = 2, TEXTURE_NORMAL = 3,
                 TEXTURE_HEIGHT = 4;
static const float BLOOM_THRESHOLD = 0.3f;
static const int MIN_SAMPLE_COUNT = 1;
static const int MAX_SAMPLE_COUNT = 8;

static const float SHADOW_BIAS = 0.05f;
static const float SHADOW_PCR_SAMPLES = 20;
static const float3 SAMPLE_OFFSET_DIRECTIONS[SHADOW_PCR_SAMPLES] = {
    float3(1.0f, 1.0f, 1.0f),    float3(1.0f, -1.0f, 1.0f),
    float3(-1.0f, -1.0f, 1.0f),  float3(-1.0f, 1.0f, 1.0f),
    float3(1.0f, 1.0f, -1.0f),   float3(1.0f, -1.0f, -1.0f),
    float3(-1.0f, -1.0f, -1.0f), float3(-1.0f, 1.0f, -1.0f),
    float3(1.0f, 1.0f, 0.0f),    float3(1.0f, -1.0f, 0.0f),
    float3(-1.0f, -1.0f, 0.0f),  float3(-1.0f, 1.0f, 0.0f),
    float3(1.0f, 0.0f, 1.0f),    float3(-1.0f, 0.0f, 1.0f),
    float3(1.0f, 0.0f, -1.0f),   float3(-1.0f, 0.0f, -1.0f),
    float3(0.0f, 1.0f, 1.0f),    float3(0.0f, -1.0f, 1.0f),
    float3(0.0f, -1.0f, -1.0f),  float3(0.0f, 1.0f, -1.0f)};
static const float FAR_PLANE = 50.0f;

// ///////////////////////////////////////////////////////// Constant buffers //
cbuffer MaterialParameters : register(b9) { float parallaxHeight; };

cbuffer LightParameters : register(b10) {
    float4 lightPositionWorld[NUM_LIGHTS];
    float4 viewPositionWorld;

    float4 diffuseColor[NUM_LIGHTS];

    float4 intensity[NUM_LIGHTS / 4];
    float4 attenuationConstant[NUM_LIGHTS / 4];
    float4 attenuationLinear[NUM_LIGHTS / 4];
    float4 attenuationQuadratic[NUM_LIGHTS / 4];

    float4 mainLightPos;
};

// /////////////////////////////////////////////////////////// Normal mapping //
float3 calculateMappedNormal(PixelShaderInput input, float2 texCoords,
                             float3x3 tangentToWorld) {
    return normalize(mul(
        (2.0f * textures[TEXTURE_NORMAL].Sample(textureSampler, texCoords).xyz -
         1.0f),
        tangentToWorld));
}

// /////////////////////////////////////////////// Physically Based Rendering //
float distributionGGX(float3 n, float3 h, float roughness) {
    float a = pow(roughness, 4.0f);

    return a / (PI *
                pow(pow(max(dot(n, h), 0.0f), 2.0f) * (a - 1.0f) + 1.0f, 2.0f));
}

float geometrySchlickGGX(float nDotV, float roughness) {
    float k = pow((roughness + 1.0f), 2.0f) / 8.0f;

    return nDotV / (nDotV * (1.0f - k) + k);
}

float geometrySmith(float3 n, float3 v, float3 l, float roughness) {
    return geometrySchlickGGX(max(dot(n, l), 0.0f), roughness) *
           geometrySchlickGGX(max(dot(n, v), 0.0f), roughness);
}

float3 fresnelSchlick(float cosTheta, float3 f0) {
    return f0 + (1.0f - f0) * pow(1.0f - cosTheta, 5.0f);
}

float attenuate(float distance, int iterator) {
    return 1.0f / (((float[NUM_LIGHTS])attenuationConstant)[iterator] +
                   ((float[NUM_LIGHTS])attenuationLinear)[iterator] * distance +
                   ((float[NUM_LIGHTS])attenuationQuadratic)[iterator] *
                       pow(distance, 2.0f));
    /*return 1.0f / (0.0f +
                   0.0f * distance +
                   0.0f *
                       pow(distance, 2.0f));*/
}

float4 pbr(PixelShaderInput input, float3 normal, float2 texCoord) {
    // Load texture parameters
    float3 albedo =
        pow(textures[TEXTURE_ALBEDO].Sample(textureSampler, texCoord).rgb,
            2.2f * float3(1.0f, 1.0f, 1.0f));
    float ao =
        textures[TEXTURE_AMBIENT_OCCLUSION].Sample(textureSampler, texCoord).r;
    float metalness = textures[TEXTURE_METALLIC_SMOOTHNESS]
                          .Sample(textureSampler, texCoord)
                          .r;
    float roughness = 1.0f - textures[TEXTURE_METALLIC_SMOOTHNESS]
                                 .Sample(textureSampler, texCoord)
                                 .a;

    // Calculate view direction
    float3 viewDir = normalize(viewPositionWorld.xyz - input.positionWorld);
    float4 finalColor = {0.0f, 0.0f, 0.0f, 0.0f};

    // Radiance
    for (int i = 0; i < NUM_LIGHTS; ++i) {
        // Shadow
        float shadow = 1.0f;
        if (i == 0) {
            const float3 shadowDepthDir = input.positionWorld - mainLightPos;

            const float currentDepth = length(shadowDepthDir);

            const float viewDistance =
                length(viewPositionWorld - input.positionWorld);
            const float diskRadius =
                (1.0f + (viewDistance / FAR_PLANE)) / 20.0f;

            for (int i = 0; i < SHADOW_PCR_SAMPLES; ++i) {
                const float closestDepth = shadowMap.Sample(
                    textureSampler,
                    normalize(shadowDepthDir +
                              SAMPLE_OFFSET_DIRECTIONS[i] * diskRadius));
                shadow +=
                    currentDepth - SHADOW_BIAS > closestDepth ? 0.2f : 1.0f;
            }
            shadow /= float(SHADOW_PCR_SAMPLES);
        }

        float3 lightDir =
            normalize(lightPositionWorld[i].xyz - input.positionWorld);
        float3 h = normalize(viewDir + lightDir);
        float factor =
            ((float[NUM_LIGHTS])intensity)[i] *
            attenuate(length(lightPositionWorld[i].xyz - input.positionWorld),
                      i);
        float3 radiance = diffuseColor[i].xyz * factor;

        // Cook-Torrance BRDF
        float ndf = distributionGGX(normal, h, roughness);
        float g = geometrySmith(normal, viewDir, lightDir, roughness);
        float3 f = fresnelSchlick(
            max(dot(h, viewDir), 0.0f),
            lerp(0.04f * float3(1.0f, 1.0f, 1.0f), albedo, metalness));

        float3 kD = (float3(1.0, 1.0f, 1.0f) - f) * (1.0f - metalness);

        float3 specular =
            (ndf * g * f) / max((4.0f * max(dot(normal, viewDir), 0.0f) *
                                 max(dot(normal, lightDir), 0.0f)),
                                0.001f);
        finalColor += float4((kD * albedo / PI + specular) * radiance *
                                 max(dot(normal, lightDir), 0.0f),
                             1.0f) *
                      shadow;
    }
    return finalColor;
}

// ////////////////////////////////////////////////////////////// Light types //
float4 pointLight(PixelShaderInput input, float3 normal, float2 texCoord) {
    return pbr(input, normal, texCoord);
}

// /////////////////////////////////////////////// Parallax occlusion mapping //
float2 parallaxOcclusionMapping(PixelShaderInput input, float2 texCoords,
                                float3 viewDirectionTangent,
                                float3 viewDirectionWorld) {
    const float2 maxParallaxOffset =
        -viewDirectionTangent.xy * parallaxHeight / viewDirectionTangent.z;

    const int sampleCount =
        (int)lerp(MAX_SAMPLE_COUNT, MIN_SAMPLE_COUNT,
                  dot(viewDirectionWorld, input.normalWorld));

    const float depthStep = 1.0f / float(sampleCount);
    const float2 deltaTexCoords = maxParallaxOffset * depthStep;

    const float2 dx = ddx(texCoords);
    const float2 dy = ddy(texCoords);

    int sampleIndex = 0;

    float2 currentTexOffset = 0;
    float2 previousTexOffset = 0;
    float2 finalTexOffset = 0;

    float currentRayZ = 1.0f - depthStep;
    float prevRayZ = 1.0f;

    float currentHeight = 0.0f;
    float prevHeight = 0.0f;

    while (sampleIndex < sampleCount + 1) {
        currentHeight = textures[TEXTURE_HEIGHT]
                            .SampleGrad(textureSampler,
                                        texCoords + currentTexOffset, dx, dy)
                            .r;

        // Did we cross the height profile?
        if (currentHeight > currentRayZ) {
            // Do ray/line segment intersection
            // and compute the final texture offset
            float t = (prevHeight - prevRayZ) /
                      (prevHeight - currentHeight + currentRayZ - prevRayZ);
            finalTexOffset = previousTexOffset + t * deltaTexCoords;

            // Exit loop.
            sampleIndex = sampleCount + 1;
        } else {
            ++sampleIndex;
            previousTexOffset = currentTexOffset;
            prevRayZ = currentRayZ;
            prevHeight = currentHeight;
            currentTexOffset += deltaTexCoords;

            // Negative because we are going "deeper" into the surface.
            currentRayZ -= depthStep;
        }
    }

    return texCoords + finalTexOffset;
}

// ///////////////////////////////////////////////////////////////////// Main //
PixelShaderOutput main(PixelShaderInput input) {
    PixelShaderOutput output;

    // Construct transform matrices between world and tangent space
    input.normalWorld = normalize(input.normalWorld);
    input.tangentWorld = normalize(input.tangentWorld -
                                   dot(input.tangentWorld, input.normalWorld) *
                                       input.normalWorld);
    input.bitangentWorld =
        normalize(cross(input.normalWorld, input.tangentWorld));

    float3x3 tangentToWorld =
        float3x3(input.tangentWorld, input.bitangentWorld, input.normalWorld);
    float3x3 worldToTangent = transpose(tangentToWorld);

    // Update texture coordinates with parallax mapping
    float3 viewDirectionWorld =
        normalize(viewPositionWorld.xyz - input.positionWorld);
    float3 viewDirectionTangent =
        normalize(mul(-viewDirectionWorld.xyz, worldToTangent));

    float2 texCoordParallax = input.texCoord;
    if (parallaxHeight > 0.005f) {
        texCoordParallax = parallaxOcclusionMapping(
            input, input.texCoord, viewDirectionTangent, viewDirectionWorld);
    }

    // Update normal with normal mapping
    float3 normal =
        calculateMappedNormal(input, texCoordParallax,
                              float3x3(input.tangentWorld,
                                       -input.bitangentWorld,
                                       input.normalWorld));

    // Calculate lighting
    output.color =
        clamp(pointLight(input, normal, texCoordParallax),
              float4(0.0f, 0.0f, 0.0f, 0.0f), float4(1.0f, 1.0f, 1.0f, 1.0f));
    // Calculate final pixel color
    float4 pixelColor = float4(textures[TEXTURE_AMBIENT_OCCLUSION]
                                       .Sample(textureSampler, texCoordParallax)
                                       .rgb *
                                   output.color.rgb,
                               1.0f);

    output.color = pixelColor;
    output.bloom =
        saturate((output.color - BLOOM_THRESHOLD) / (1 - BLOOM_THRESHOLD));

    float fog =
        1.0f /
        (pow(0.01f * distance(input.positionWorld, viewPositionWorld), 2) +
         1.0f);

    output.color = lerp(output.color + 0.05f, output.color, fog);

    return output;
}

// ////////////////////////////////////////////////////////////////////////// //

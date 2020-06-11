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

// ///////////////////////////////////////////////////////////////// Texture //
Texture2D textures[5];
TextureCube skybox : register(t6);
SamplerState textureSampler {
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

// //////////////////////////////////////////////////////////////// Constants //
static const float REFRACT_FACTOR = 1.0f / 1.33f;
static const float BLOOM_THRESHOLD = 0.1f;
static const int TEXTURE_ALBEDO = 0, TEXTURE_AMBIENT_OCCLUSION = 1,
                 TEXTURE_METALLIC_SMOOTHNESS = 2, TEXTURE_NORMAL = 3,
                 TEXTURE_HEIGHT = 4;
static const int MIN_SAMPLE_COUNT = 4;
static const int MAX_SAMPLE_COUNT = 32;
static const int NUM_LIGHTS = 16;

// ///////////////////////////////////////////////////////// Constant buffers //
cbuffer MaterialParameters : register(b9) { float parallaxHeight; };

cbuffer LightParameters : register(b10) {
    float4 lightPositionWorld[NUM_LIGHTS];
    float4 viewPositionWorld;

    float4 diffuseColor[NUM_LIGHTS];

    float4 intensity[NUM_LIGHTS / 4];

    float attenuationConstant;
    float attenuationLinear;
    float attenuationQuadratic;
};

cbuffer AnimationSpeed : register(b8) { float animationSpeed = 0.0f; }

// /////////////////////////////////////////////////////////// Normal mapping //
float3 calculateMappedNormal(PixelShaderInput input, float2 texCoords,
                             float3x3 tangentToWorld) {
    return normalize(mul(
        (2.0f * textures[TEXTURE_NORMAL].Sample(textureSampler, texCoords).xyz -
         1.0f),
        tangentToWorld));
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
                              float3x3(normalize(input.tangentWorld),
                                       -normalize(input.bitangentWorld),
                                       normalize(input.normalWorld)));

    // Calculate the refraction
    float3 refractionDirection =
        normalize(input.positionWorld - viewPositionWorld);

    if (animationSpeed > 0.0f) {
        // Calculate additional cubemap rotation
        float theta = animationSpeed * input.positionWorld.x;
        float3x3 rotationMatrix = float3x3(
            float3(cos(theta), -sin(theta), 0.0f),
            float3(sin(theta), cos(theta), 0.0f), float3(0.0f, 0.0f, 1.0f));

        refractionDirection =
            normalize(mul(refractionDirection, rotationMatrix));
    }
    
    float3 refractionCoord = refract(
        refractionDirection, normalize(lerp(normal, input.normalWorld, 0.8f)),
        REFRACT_FACTOR);

    PixelShaderOutput output;
    output.color = skybox.Sample(textureSampler, refractionCoord);
    output.bloom =
        saturate((output.color - BLOOM_THRESHOLD) / (1 - BLOOM_THRESHOLD));
    return output;
}

// ////////////////////////////////////////////////////////////////////////// //

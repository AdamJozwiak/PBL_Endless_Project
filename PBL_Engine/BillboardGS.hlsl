cbuffer CBuf {
    matrix model;
    matrix viewProj;
    float4 cameraPos;
    float textureWidth;
    float textureHeight;
};

struct VSOut {
    float4 pos : SV_POSITION;
    float4 worldPos : POSITION;
    float2 tex : TEXCOORD;
};

[maxvertexcount(4)] void main(point VSOut input[1],
                              inout TriangleStream<VSOut> OutputStream) {
    matrix modelViewProj = mul(model, viewProj);
    float halfWidth = textureWidth / 2.0f;

    float3 planeNormal = input[0].worldPos - cameraPos;
    planeNormal.y = 0.0f;
    planeNormal = normalize(planeNormal);

    float3 upVector = float3(0.0f, 1.0f, 0.0f);

    float3 rightVector = normalize(cross(planeNormal, upVector));

    rightVector = rightVector * halfWidth;

    upVector = float3(0, textureHeight, 0);

    // Create the billboards quad
    float3 vert[4];

    vert[0] = input[0].worldPos - rightVector;  // Get bottom left vertex
    vert[1] = input[0].worldPos + rightVector;  // Get bottom right vertex
    vert[2] =
        input[0].worldPos - rightVector + upVector;  // Get top left vertex
    vert[3] =
        input[0].worldPos + rightVector + upVector;  // Get top right vertex

    // Get billboards texture coordinates
    float2 texCoord[4];
    texCoord[0] = float2(0, 1);
    texCoord[1] = float2(1, 1);
    texCoord[2] = float2(0, 0);
    texCoord[3] = float2(1, 0);

    VSOut outputVert;
    for (int i = 0; i < 4; i++) {
        outputVert.pos = mul(float4(vert[i], 1.0f), modelViewProj);
        outputVert.worldPos = float4(vert[i], 0.0f);
        outputVert.tex = texCoord[i];
        OutputStream.Append(outputVert);
    }
}
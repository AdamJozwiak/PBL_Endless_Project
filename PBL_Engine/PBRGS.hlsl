// /////////////////////////////////////////////////////// Inputs and outputs //
struct GeometryShaderInputOutput {
    float3 positionWorld : POSITION;
    float3 normalWorld : NORMAL;
    float3 tangentWorld : TANGENT;
    float2 texCoord : TEXCOORD;
    float4 position : SV_POSITION;
};

// ///////////////////////////////////////////////////////////////////// Main //
[maxvertexcount(3)] void main(
    triangle GeometryShaderInputOutput input[3]
    : SV_POSITION, inout TriangleStream<GeometryShaderInputOutput> output) {
    for (uint i = 0; i < 3; i++) {
        GeometryShaderInputOutput element;
        element.positionWorld = input[i].positionWorld;
        element.normalWorld = input[i].normalWorld;
        element.tangentWorld = input[i].tangentWorld;
        element.texCoord = input[i].texCoord;
        element.position = input[i].position;

        output.Append(element);
    }
}

// ////////////////////////////////////////////////////////////////////////// //

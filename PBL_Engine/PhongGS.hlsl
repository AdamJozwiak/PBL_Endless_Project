struct VertexShaderOutput {
    float3 worldPos : Position;
    float3 normal : Normal;
    float4 position : SV_Position;
};

[maxvertexcount(3)] void main(
    triangle VertexShaderOutput input[3]
    : SV_POSITION, inout TriangleStream<VertexShaderOutput> output) {
    for (uint i = 0; i < 3; i++) {
        VertexShaderOutput element;
        element.worldPos = input[i].worldPos;
        element.normal = input[i].normal;
        element.position = input[i].position;
        output.Append(element);
    }
}
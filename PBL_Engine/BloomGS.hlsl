struct GSOutput {
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD;
};

struct VSOut {
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD;
};

[maxvertexcount(3)] void main(triangle VSOut input[3]
                              : SV_POSITION,
                                inout TriangleStream<GSOutput> output) {
    for (uint i = 0; i < 3; i++) {
        GSOutput element;
        element.pos = input[i].pos;
        element.tex = input[i].tex;
        output.Append(element);
    }
}
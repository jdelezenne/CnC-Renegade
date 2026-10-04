struct PresentOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};
PresentOutput VertexMain(uint index : SV_VertexID) {
    PresentOutput output;
    output.uv = float2((index << 1) & 2, index & 2);
    output.position = float4(output.uv * float2(2, -2) + float2(-1, 1), 0, 1);
    return output;
}
Texture2D<float4> image : register(t0, space2);
Texture2D<float4> gammaRamp : register(t1, space2);
SamplerState imageSampler : register(s0, space2);
SamplerState gammaSampler : register(s1, space2);
float4 FragmentMain(PresentOutput input) : SV_Target0 {
    float4 color = image.Sample(imageSampler, input.uv);
    float3 coordinate = saturate(color.rgb) * 255;
    int3 low = (int3)floor(coordinate);
    int3 high = min(low + 1, 255);
    float3 a = float3(gammaRamp.Load(int3(low.r, 0, 0)).r,
        gammaRamp.Load(int3(low.g, 0, 0)).g, gammaRamp.Load(int3(low.b, 0, 0)).b);
    float3 b = float3(gammaRamp.Load(int3(high.r, 0, 0)).r,
        gammaRamp.Load(int3(high.g, 0, 0)).g, gammaRamp.Load(int3(high.b, 0, 0)).b);
    return float4(lerp(a, b, frac(coordinate)), color.a);
}

struct FragmentInput {
    float4 position : SV_Position;
    float4 diffuse : COLOR0;
    float4 specular : COLOR1;
    float4 uv0 : TEXCOORD0;
    float4 uv1 : TEXCOORD1;
    float4 uv2 : TEXCOORD2;
    float4 uv3 : TEXCOORD3;
    float4 uv4 : TEXCOORD4;
    float4 uv5 : TEXCOORD5;
    float4 uv6 : TEXCOORD6;
    float4 uv7 : TEXCOORD7;
    float fog : TEXCOORD8;
};
Texture2D<float4> texture0 : register(t0, space2);
Texture2D<float4> texture1 : register(t1, space2);
Texture2D<float4> texture2 : register(t2, space2);
Texture2D<float4> texture3 : register(t3, space2);
Texture2D<float4> texture4 : register(t4, space2);
Texture2D<float4> texture5 : register(t5, space2);
Texture2D<float4> texture6 : register(t6, space2);
Texture2D<float4> texture7 : register(t7, space2);
SamplerState sampler0 : register(s0, space2);
SamplerState sampler1 : register(s1, space2);
SamplerState sampler2 : register(s2, space2);
SamplerState sampler3 : register(s3, space2);
SamplerState sampler4 : register(s4, space2);
SamplerState sampler5 : register(s5, space2);
SamplerState sampler6 : register(s6, space2);
SamplerState sampler7 : register(s7, space2);
struct Stage {
    int4 color;
    int4 alpha;
    int4 state;
    float4 sampling;
    float4 bumpMatrix;
    float4 border;
};
cbuffer FragmentState : register(b0, space3) {
    float4 factor;
    float4 fogColor;
    float4 fogParameters;
    int4 flags;
    float4 alphaParameters;
    Stage stages[8];
};
float4 SampleTexture(int stage, float2 uv, float bias) {
    switch (stage) {
        case 0: return texture0.SampleBias(sampler0, uv, bias);
        case 1: return texture1.SampleBias(sampler1, uv, bias);
        case 2: return texture2.SampleBias(sampler2, uv, bias);
        case 3: return texture3.SampleBias(sampler3, uv, bias);
        case 4: return texture4.SampleBias(sampler4, uv, bias);
        case 5: return texture5.SampleBias(sampler5, uv, bias);
        case 6: return texture6.SampleBias(sampler6, uv, bias);
        default: return texture7.SampleBias(sampler7, uv, bias);
    }
}
float4 Argument(int selection, float4 texel, float4 diffuse, float4 current, float4 specular, float4 temporary) {
    float4 result;
    switch (selection & 15) {
        case 0: result = diffuse; break;
        case 1: result = current; break;
        case 2: result = texel; break;
        case 3: result = factor; break;
        case 4: result = specular; break;
        default: result = temporary; break;
    }
    if ((selection & 32) != 0) result = result.aaaa;
    if ((selection & 16) != 0) result = 1 - result;
    return result;
}
float4 Combine(int operation, float4 a, float4 b, float4 c, float4 diffuse, float4 texel, float4 current) {
    switch (operation) {
        case 2: return a;
        case 3: return b;
        case 4: return a * b;
        case 5: return 2 * a * b;
        case 6: return 4 * a * b;
        case 7: return a + b;
        case 8: return a + b - 0.5;
        case 9: return 2 * (a + b - 0.5);
        case 10: return a - b;
        case 11: return a + b * (1 - a);
        case 12: return lerp(b, a, diffuse.a);
        case 13: return lerp(b, a, texel.a);
        case 14: return lerp(b, a, factor.a);
        case 15: return a + b * (1 - texel.a);
        case 16: return lerp(b, a, current.a);
        case 17: return a * b;
        case 18: return a + a.a * b;
        case 19: return a * b + a.a;
        case 20: return a + (1 - a.a) * b;
        case 21: return (1 - a) * b + a.a;
        case 24: return dot(a.rgb * 2 - 1, b.rgb * 2 - 1).xxxx;
        case 25: return a * b + c;
        case 26: return lerp(b, a, c);
        default: return current;
    }
}
bool CompareAlpha(float a, float b, int comparison) {
    switch (comparison) {
        case 1: return false;
        case 2: return a < b;
        case 3: return a == b;
        case 4: return a <= b;
        case 5: return a > b;
        case 6: return a != b;
        case 7: return a >= b;
        default: return true;
    }
}
float4 main(FragmentInput input) : SV_Target0 {
    float4 coordinates[8] = { input.uv0, input.uv1, input.uv2, input.uv3, input.uv4, input.uv5, input.uv6, input.uv7 };
    float4 current = input.diffuse;
    float4 temporary = 0;
    float2 bumpOffset = 0;
    float bumpLuminance = 1;
    float4 premodulate = 1;
    for (int i = 0; i < 8; ++i) {
        Stage stage = stages[i];
        if (stage.color.x == 1) break;
        float4 coordinate = coordinates[i];
        if ((stage.state.y & 256) != 0) {
            int count = stage.state.y & 15;
            coordinate.xy /= count == 3 ? coordinate.z : coordinate.w;
        }
        float2 uv = coordinate.xy + bumpOffset;
        bumpOffset = 0;
        if (stage.state.z == 5) uv.x = saturate(abs(uv.x));
        if (stage.state.w == 5) uv.y = saturate(abs(uv.y));
        bool border = (stage.state.z == 4 && (uv.x < 0 || uv.x > 1)) || (stage.state.w == 4 && (uv.y < 0 || uv.y > 1));
        float4 texel = border ? stage.border : SampleTexture(i, uv, stage.sampling.x);
        texel *= premodulate;
        premodulate = 1;
        texel.rgb *= bumpLuminance;
        bumpLuminance = 1;
        if (stage.color.x == 22 || stage.color.x == 23) {
            bumpOffset = float2(dot(texel.rg, stage.bumpMatrix.xy), dot(texel.rg, stage.bumpMatrix.zw));
            if (stage.color.x == 23) bumpLuminance = texel.b * stage.sampling.z + stage.sampling.w;
            continue;
        }
        float4 color = Combine(stage.color.x,
            Argument(stage.color.y, texel, input.diffuse, current, input.specular, temporary),
            Argument(stage.color.z, texel, input.diffuse, current, input.specular, temporary),
            Argument(stage.color.w, texel, input.diffuse, current, input.specular, temporary), input.diffuse, texel, current);
        float alpha = stage.alpha.x == 1 ? current.a : Combine(stage.alpha.x,
            Argument(stage.alpha.y, texel, input.diffuse, current, input.specular, temporary),
            Argument(stage.alpha.z, texel, input.diffuse, current, input.specular, temporary),
            Argument(stage.alpha.w, texel, input.diffuse, current, input.specular, temporary), input.diffuse, texel, current).a;
        float4 result = saturate(float4(color.rgb, alpha));
        if (stage.state.x == 5) temporary = result;
        else current = result;
        if (stage.color.x == 17) premodulate = texel;
    }
    if (flags.x != 0 && !CompareAlpha(round(saturate(current.a) * 255), alphaParameters.x, flags.y)) discard;
    if (flags.z != 0) current.rgb = saturate(current.rgb + input.specular.rgb);
    if (flags.w != 0) {
        float f = 1;
        int mode = (int)fogParameters.w;
        float distance = alphaParameters.y != 0 ? input.position.z : input.fog;
        if (mode < 0) f = input.fog;
        if (mode == 1) f = exp(-fogParameters.z * distance);
        if (mode == 2) f = exp(-pow(fogParameters.z * distance, 2));
        if (mode == 3) f = (fogParameters.y - distance) / (fogParameters.y - fogParameters.x);
        current.rgb = lerp(fogColor.rgb, current.rgb, saturate(f));
    }
    return current;
}

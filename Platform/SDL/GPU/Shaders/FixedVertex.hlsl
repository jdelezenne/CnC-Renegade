struct VertexInput {
    float4 position : TEXCOORD0;
    float4 normal : TEXCOORD1;
    float4 diffuse : TEXCOORD2;
    float4 specular : TEXCOORD3;
    float4 uv0 : TEXCOORD4;
    float4 uv1 : TEXCOORD5;
    float4 uv2 : TEXCOORD6;
    float4 uv3 : TEXCOORD7;
    float4 uv4 : TEXCOORD8;
    float4 uv5 : TEXCOORD9;
    float4 uv6 : TEXCOORD10;
    float4 uv7 : TEXCOORD11;
};
struct VertexOutput {
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
struct Light {
    float4 position;
    float4 direction;
    float4 diffuse;
    float4 ambient;
    float4 specular;
    float4 parameters;
    float4 settings;
};
cbuffer VertexState : register(b0, space1) {
    row_major float4x4 world;
    row_major float4x4 view;
    row_major float4x4 projection;
    row_major float4x4 normalTransform;
    row_major float4x4 textureTransforms[8];
    float4 viewport;
    int4 flags;
    float4 materialDiffuse;
    float4 materialAmbient;
    float4 materialSpecular;
    float4 materialEmissive;
    float4 ambient;
    float4 materialSettings;
    int4 materialSources;
    int4 coordinateState[8];
    Light lights[8];
    float4 fogParameters;
    int4 fogSettings;
};
float4 MaterialColor(int source, float4 material, VertexInput vertex) {
    if (materialSettings.z == 0 || source == 0) return material;
    return source == 1 ? vertex.diffuse : vertex.specular;
}
VertexOutput main(VertexInput vertex) {
    VertexOutput output;
    float4 worldPosition = mul(float4(vertex.position.xyz, 1), world);
    float4 viewPosition = mul(worldPosition, view);
    float3 normal = mul(float4(vertex.normal.xyz, 0), normalTransform).xyz;
    normal = mul(float4(normal, 0), view).xyz;
    if (flags.w != 0) normal = normalize(normal);
    if (flags.x != 0) {
        float reciprocalW = vertex.position.w;
        float w = reciprocalW != 0 ? 1 / reciprocalW : 1;
        output.position = float4((vertex.position.x + 0.5 - viewport.z) * 2 / viewport.x - 1,
            1 - (vertex.position.y + 0.5 - viewport.w) * 2 / viewport.y, vertex.position.z, 1) * w;
    } else {
        output.position = mul(viewPosition, projection);
        output.position.xy += float2(1 / viewport.x, -1 / viewport.y) * output.position.w;
    }
    output.diffuse = vertex.diffuse;
    output.specular = vertex.specular;
    if (flags.y != 0 && flags.x == 0) {
        float4 diffuse = MaterialColor(materialSources.x, materialDiffuse, vertex);
        float4 ambientMaterial = MaterialColor(materialSources.y, materialAmbient, vertex);
        float4 specular = MaterialColor(materialSources.z, materialSpecular, vertex);
        float4 emissive = MaterialColor(materialSources.w, materialEmissive, vertex);
        float3 diffuseSum = emissive.rgb + ambient.rgb * ambientMaterial.rgb;
        float3 specularSum = 0;
        float3 eye = materialSettings.y != 0 ? normalize(-viewPosition.xyz) : float3(0, 0, -1);
        for (int i = 0; i < 8; ++i) {
            Light light = lights[i];
            int type = (int)light.settings.w;
            if (type == 0) continue;
            float3 direction;
            float attenuation = 1;
            if (type == 3) direction = normalize(-light.direction.xyz);
            else {
                direction = light.position.xyz - viewPosition.xyz;
                float distance = length(direction);
                if (distance > light.parameters.x) continue;
                direction /= max(distance, 0.000001);
                attenuation = 1 / max(light.parameters.z + light.parameters.w * distance
                    + light.settings.x * distance * distance, 0.000001);
                if (type == 2) {
                    float rho = dot(-direction, normalize(light.direction.xyz));
                    float cone = saturate((rho - light.settings.z) / max(light.settings.y - light.settings.z, 0.000001));
                    attenuation *= pow(cone, light.parameters.y);
                }
            }
            float nDotL = max(dot(normal, direction), 0);
            diffuseSum += attenuation * (light.ambient.rgb * ambientMaterial.rgb + nDotL * light.diffuse.rgb * diffuse.rgb);
            if (flags.z != 0 && nDotL > 0) {
                float3 halfVector = normalize(direction + eye);
                specularSum += attenuation * pow(max(dot(normal, halfVector), 0), materialSettings.x) * light.specular.rgb * specular.rgb;
            }
        }
        output.diffuse = float4(saturate(diffuseSum), diffuse.a);
        output.specular = float4(saturate(specularSum), vertex.specular.a);
    }
    float4 coordinates[8] = { vertex.uv0, vertex.uv1, vertex.uv2, vertex.uv3, vertex.uv4, vertex.uv5, vertex.uv6, vertex.uv7 };
    float4 transformed[8];
    for (int i = 0; i < 8; ++i) {
        int index = coordinateState[i].x;
        int generation = index & 0xffff0000;
        float4 coordinate = coordinates[index & 7];
        if (generation == 0x10000) coordinate = float4(normal, 1);
        if (generation == 0x20000) coordinate = float4(viewPosition.xyz, 1);
        if (generation == 0x30000) coordinate = float4(reflect(normalize(viewPosition.xyz), normal), 1);
        if (coordinateState[i].y != 0) coordinate = mul(coordinate, textureTransforms[i]);
        transformed[i] = coordinate;
    }
    output.uv0 = transformed[0]; output.uv1 = transformed[1];
    output.uv2 = transformed[2]; output.uv3 = transformed[3];
    output.uv4 = transformed[4]; output.uv5 = transformed[5];
    output.uv6 = transformed[6]; output.uv7 = transformed[7];
    if (fogSettings.x != 0) {
        output.fog = output.position.w;
    } else {
        output.fog = vertex.specular.a;
        if (flags.x == 0 && fogSettings.y != 0) {
            float distance = materialSettings.w != 0 ? length(viewPosition.xyz) : abs(viewPosition.z);
            if (fogSettings.y == 1) output.fog = exp(-fogParameters.z * distance);
            if (fogSettings.y == 2) output.fog = exp(-pow(fogParameters.z * distance, 2));
            if (fogSettings.y == 3) output.fog = (fogParameters.y - distance) / (fogParameters.y - fogParameters.x);
            output.fog = saturate(output.fog);
        }
    }
    return output;
}

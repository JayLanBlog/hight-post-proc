#version 460
// Oil Paint (Kuwahara) — AUS Vol.10
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
    int radius = int(clamp(uParamFloat0, 1, 10));
    vec2 step = 1.0 / uResolution;
    vec4 bestColor = vec4(0);
    float bestVar = 1e10;
    for (int q = 0; q < 4; q++) {
        float sx = (q % 2 == 0) ? -1.0 : 1.0;
        float sy = (q / 2 == 0) ? -1.0 : 1.0;
        vec3 sum = vec3(0), sumSq = vec3(0);
        float cnt = 0;
        for (int dy = 1; dy <= radius; dy++) {
            for (int dx = 1; dx <= radius; dx++) {
                vec2 uv = vUV + vec2(float(dx)*sx, float(dy)*sy) * step;
                uv = clamp(uv, 0, 1);
                vec3 col = texture(uInputTex, uv).rgb;
                sum += col; sumSq += col * col; cnt += 1;
            }
        }
        vec3 mean = sum / cnt;
        float variance = dot(sumSq/cnt - mean*mean, vec3(1));
        if (variance < bestVar) {
            bestVar = variance;
            bestColor = vec4(mean, 1);
        }
    }
    outColor = bestColor;
}

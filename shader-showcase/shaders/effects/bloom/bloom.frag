#version 460
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;

layout(binding=0) uniform sampler2D uInputTex;

layout(std140, binding=1) uniform Params {
    float uParamFloat0;
    float uParamFloat1;
    float uParamFloat2;
    float uParamFloat3;
    float uParamFloat4;
    float uParamFloat5;
    vec2 uResolution;
    float uTime;
    float uFrameCount;
};

void main() {
    vec3 color = texture(uInputTex, vUV).rgb;

    float BloomIntensity = uParamFloat0;
    float Threshold = uParamFloat1;
    float BlurSize = max(uParamFloat2, 0.1);

    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float bright = max(luma - Threshold, 0.0);

    if (bright <= 0.0 || BloomIntensity <= 0.0) {
        outColor = vec4(color, 1.0);
        return;
    }

    vec2 texelSize = 1.0 / uResolution;
    float total = 0.0;
    vec3 blurred = vec3(0.0);
    
    float sigma = BlurSize * 0.5;
    float sigma2 = 2.0 * sigma * sigma;

    for (int x = -6; x <= 6; x++) {
        for (int y = -6; y <= 6; y++) {
            float dist2 = float(x * x + y * y);
            float w = exp(-dist2 / sigma2);
            vec2 offset = vec2(float(x), float(y)) * texelSize * BlurSize * 0.3;
            blurred += texture(uInputTex, vUV + offset).rgb * w;
            total += w;
        }
    }
    blurred /= max(total, 0.001);
    blurred *= bright;

    outColor = vec4(color + blurred * BloomIntensity, 1.0);
}

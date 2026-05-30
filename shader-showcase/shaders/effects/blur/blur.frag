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

    float BlurRadius = max(uParamFloat0, 0.5); // Prevent divide by zero
    float BlurStrength = uParamFloat1;

    // Early exit if no blur
    if (BlurStrength <= 0.0) {
        outColor = vec4(color, 1.0);
        return;
    }

    // Scale effective radius with strength for responsive slider feel
    float effectiveRadius = BlurRadius * BlurStrength;

    // 9-tap approximate Gaussian blur
    vec2 texelSize = 1.0 / uResolution;
    vec3 result = vec3(0.0);
    float total = 0.0;
    
    float sigma = effectiveRadius * 0.5;
    float sigma2 = 2.0 * sigma * sigma;

    for (int x = -4; x <= 4; x++) {
        for (int y = -4; y <= 4; y++) {
            float dist2 = float(x * x + y * y);
            float w = exp(-dist2 / sigma2);
            vec2 offset = vec2(float(x), float(y)) * texelSize * effectiveRadius * 0.3;
            result += texture(uInputTex, vUV + offset).rgb * w;
            total += w;
        }
    }
    result /= max(total, 0.001);

    outColor = vec4(mix(color, result, BlurStrength), 1.0);
}

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

    float Amount = uParamFloat0;
    float Radius = uParamFloat1;

    // Early exit if no sharpening
    if (Amount <= 0.0) {
        outColor = vec4(color, 1.0);
        return;
    }

    // Gaussian-weighted radius-dependent blur (3x3 kernel)
    // Larger radius = wider blur = more sharpening effect
    vec2 texelSize = 1.0 / uResolution;
    float sigma = Radius * 0.6;
    float sigma2 = 2.0 * sigma * sigma;
    float total = 0.0;
    vec3 blur = vec3(0.0);

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float dist2 = float(x * x + y * y);
            float w = exp(-dist2 / sigma2);
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            blur += texture(uInputTex, vUV + offset).rgb * w;
            total += w;
        }
    }
    blur /= max(total, 0.001);

    // Unsharp mask: original + amount * (original - blur)
    vec3 sharpened = color + (color - blur) * Amount;

    outColor = vec4(clamp(sharpened, 0.0, 1.0), 1.0);
}

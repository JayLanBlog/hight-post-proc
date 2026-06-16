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
    float BlurSize = max(uParamFloat2, 1.0);

    // Early exit if no bloom
    if (BloomIntensity <= 0.0) {
        outColor = vec4(color, 1.0);
        return;
    }

    vec2 texelSize = 1.0 / uResolution;
    
    // BlurSize controls kernel range for dramatic visual effect
    int range = int(ceil(BlurSize));
    float sigma = BlurSize;
    float sigma2 = 2.0 * sigma * sigma;
    
    vec3 bloom = vec3(0.0);
    float weightSum = 0.0;

    for (int x = -range; x <= range; x++) {
        for (int y = -range; y <= range; y++) {
            // Gaussian weight
            float dist2 = float(x * x + y * y);
            float w = exp(-dist2 / sigma2);
            
            // Sample offset scaled by BlurSize
            vec2 offset = vec2(float(x), float(y)) * texelSize * BlurSize;
            vec3 sampleColor = texture(uInputTex, vUV + offset).rgb;
            
            // Threshold filtering: only bright pixels contribute
            float sampleLuma = dot(sampleColor, vec3(0.2126, 0.7152, 0.0722));
            if (sampleLuma > Threshold) {
                // Brightness above threshold contributes to bloom
                float brightness = (sampleLuma - Threshold) / (1.0 - Threshold + 0.001);
                bloom += sampleColor * brightness * w;
            }
            weightSum += w;
        }
    }
    bloom /= max(weightSum, 0.001);

    outColor = vec4(color + bloom * BloomIntensity, 1.0);
}

#version 460
// 油画特效：Kuwahara滤波器 — 2象限方差最小选择
// Reference: Volume 10 ScreenOilPaintEffect (ShaderToy #MsXSRN)
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

void main() {
    int radius = int(P0 * 5.0);  // P0∈[0.1,1.0], _Radius∈[0,5], default 2
    vec2 texelSize = 1.0 / vec2(textureSize(uInputTex, 0));
    vec2 srcSize = P1 * texelSize;  // _ResolutionValue * texelSize
    
    float n = float((radius + 1) * (radius + 1));
    
    vec3 m0 = vec3(0.0), m1 = vec3(0.0);
    vec3 s0 = vec3(0.0), s1 = vec3(0.0);
    vec3 c;
    
    // Quadrant 1: neg-neg (j=-radius..0, i=-radius..0)
    for (int j = -radius; j <= 0; ++j) {
        for (int i = -radius; i <= 0; ++i) {
            c = texture(uInputTex, vUV + vec2(float(i), float(j)) * srcSize).rgb;
            m0 += c;
            s0 += c * c;
        }
    }
    
    // Quadrant 2: pos-pos (j=0..radius, i=0..radius)
    for (int j = 0; j <= radius; ++j) {
        for (int i = 0; i <= radius; ++i) {
            c = texture(uInputTex, vUV + vec2(float(i), float(j)) * srcSize).rgb;
            m1 += c;
            s1 += c * c;
        }
    }
    
    vec4 finalColor = vec4(0.0);
    float minSigma2 = 1e+2;
    
    m0 /= n;
    s0 = abs(s0 / n - m0 * m0);
    float sigma2 = s0.r + s0.g + s0.b;
    if (sigma2 < minSigma2) {
        minSigma2 = sigma2;
        finalColor = vec4(m0, 1.0);
    }
    
    m1 /= n;
    s1 = abs(s1 / n - m1 * m1);
    sigma2 = s1.r + s1.g + s1.b;
    if (sigma2 < minSigma2) {
        minSigma2 = sigma2;
        finalColor = vec4(m1, 1.0);
    }
    
    outColor = finalColor;
}
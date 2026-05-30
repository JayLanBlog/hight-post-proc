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

    // Sample surrounding pixels for unsharp mask
    vec2 texelSize = 1.0 / uResolution;
    vec3 tl = texture(uInputTex, vUV + vec2(-1.0, -1.0) * texelSize * Radius).rgb;
    vec3 t  = texture(uInputTex, vUV + vec2( 0.0, -1.0) * texelSize * Radius).rgb;
    vec3 tr = texture(uInputTex, vUV + vec2( 1.0, -1.0) * texelSize * Radius).rgb;
    vec3 l  = texture(uInputTex, vUV + vec2(-1.0,  0.0) * texelSize * Radius).rgb;
    vec3 r  = texture(uInputTex, vUV + vec2( 1.0,  0.0) * texelSize * Radius).rgb;
    vec3 bl = texture(uInputTex, vUV + vec2(-1.0,  1.0) * texelSize * Radius).rgb;
    vec3 b  = texture(uInputTex, vUV + vec2( 0.0,  1.0) * texelSize * Radius).rgb;
    vec3 br = texture(uInputTex, vUV + vec2( 1.0,  1.0) * texelSize * Radius).rgb;

    // Unsharp mask: original + amount * (original - blur)
    vec3 blur = (tl + tr + bl + br) * 0.0625 + (t + l + r + b) * 0.125 + color * 0.25;
    vec3 sharpened = color + (color - blur) * Amount;

    outColor = vec4(clamp(sharpened, 0.0, 1.0), 1.0);
}

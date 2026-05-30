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

// 简化的色温转RGB (Tanner Helland算法)
vec3 kelvinToRGB(float temp) {
    float t = temp / 100.0;
    vec3 color;
    // Red
    if (t <= 66.0) color.r = 1.0;
    else color.r = clamp(1.292936 * pow(t - 60.0, -0.133204), 0.0, 1.0);
    // Green
    if (t <= 66.0) color.g = clamp(0.39008 * log(t) - 0.63184, 0.0, 1.0);
    else color.g = clamp(1.12989 * pow(t - 60.0, -0.07551), 0.0, 1.0);
    // Blue
    if (t >= 66.0) color.b = 1.0;
    else if (t <= 19.0) color.b = 0.0;
    else color.b = clamp(0.54320 * log(t - 10.0) - 0.27556, 0.0, 1.0);
    return color;
}

void main() {
    vec3 color = texture(uInputTex, vUV).rgb;

    // Exposure
    color *= exp2(uParamFloat0);

    // Contrast (around 0.18 midpoint)
    color = (color - 0.5) * (1.0 + uParamFloat1) + 0.5;

    // Saturation
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color = mix(vec3(luma), color, 1.0 + uParamFloat2);

    // Temperature
    vec3 kelvinColor = kelvinToRGB(uParamFloat3);
    vec3 kelvinNeutral = kelvinToRGB(6500.0);
    vec3 tempTint = kelvinColor / kelvinNeutral;
    color *= tempTint;

    // Tint (green-magenta shift)
    color.g += uParamFloat4 * 0.1;

    outColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}

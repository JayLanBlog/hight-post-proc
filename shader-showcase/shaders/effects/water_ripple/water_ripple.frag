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
    // 多层正弦波叠加模拟水波纹
    vec2 uv = vUV;
    float wave1 = sin(uv.x * uParamFloat1 + uTime * uParamFloat2) * uParamFloat0;
    float wave2 = sin(uv.y * uParamFloat1 * 0.8 + uTime * uParamFloat2 * 1.3) * uParamFloat0 * 0.7;
    float wave3 = sin((uv.x + uv.y) * uParamFloat1 * 0.5 + uTime * uParamFloat2 * 0.7) * uParamFloat0 * 0.5;

    // 法线近似 (偏导数)
    float dx = cos(uv.x * uParamFloat1 + uTime * uParamFloat2) * uParamFloat0 * uParamFloat1
             + cos((uv.x + uv.y) * uParamFloat1 * 0.5 + uTime * uParamFloat2 * 0.7) * uParamFloat0 * 0.5 * uParamFloat1 * 0.5;
    float dy = cos(uv.y * uParamFloat1 * 0.8 + uTime * uParamFloat2 * 1.3) * uParamFloat0 * 0.7 * uParamFloat1 * 0.8
             + cos((uv.x + uv.y) * uParamFloat1 * 0.5 + uTime * uParamFloat2 * 0.7) * uParamFloat0 * 0.5 * uParamFloat1 * 0.5;

    // 折射偏移
    vec2 refractOffset = vec2(dx, dy) * uParamFloat3;
    vec3 color = texture(uInputTex, uv + refractOffset).rgb;

    // 高光 (模拟水面反射)
    float specular = pow(max(dot(normalize(vec3(dx, dy, 1.0)), normalize(vec3(0.0, 0.0, 1.0))), 0.0), 32.0);
    color += specular * 0.15;

    // 深度感 (波峰亮波谷暗)
    float depth = (wave1 + wave2 + wave3) * 10.0;
    color += depth * 0.05;

    outColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}

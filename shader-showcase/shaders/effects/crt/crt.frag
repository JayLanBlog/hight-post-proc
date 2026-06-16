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
    vec2 uv = vUV - 0.5;
    float dist2 = dot(uv, uv);

    // 屏幕弯曲 (barrel distortion)
    vec2 curvedUV = uv * (1.0 + uParamFloat1 * dist2) + 0.5;

    // 超出范围: 柔和暗边
    vec2 clampedUV = clamp(curvedUV, 0.0, 1.0);
    float borderFade = 1.0;
    if (curvedUV.x < 0.0 || curvedUV.x > 1.0 ||
        curvedUV.y < 0.0 || curvedUV.y > 1.0) {
        vec2 excess = abs(curvedUV - 0.5) - 0.5;
        borderFade = 1.0 - clamp(max(excess.x, excess.y) * 3.0, 0.15, 0.4);
    }

    vec3 color = texture(uInputTex, clampedUV).rgb;

    // 扫描线 — 微弱可见
    float scanline = sin(curvedUV.y * uResolution.y * 3.14159) * 0.5 + 0.5;
    float scanDarken = 1.0 - scanline * uParamFloat0 * 0.25;
    color *= max(scanDarken, 0.92);

    // RGB 磷光点遮罩
    vec2 pixelPos = curvedUV * uResolution;
    float modX = mod(pixelPos.x, 3.0);
    float mask = (modX < 1.0) ? 1.0 : ((modX < 2.0) ? 0.93 : 0.87);
    color *= mix(1.0, mask, uParamFloat2);

    // 闪烁
    float flicker = 1.0 - uParamFloat4 * sin(uTime * 60.0) * 0.2;
    color *= flicker;

    // 边缘暗角 — 轻柔
    float vignette = 1.0 - dist2 * 0.35;
    color *= max(vignette, 0.65);

    color *= borderFade;

    // 亮度补偿 + 暗部提亮
    float brightness = 1.0 + uParamFloat3 * 2.0;
    color = color * brightness + 0.05;  // 暗部也加一点

    outColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}

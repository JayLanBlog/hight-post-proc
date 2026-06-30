#version 460
// Vol.04 Glass: 玻璃球体 - 高透光高光泽
// 三层高光：极窄核心 + 中等辉光 + 宽散射
// 使用uLightDir使高光在中心可见
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

bool hitSphere(vec3 ro, vec3 rd, float r, out float t) {
    float b = dot(ro, rd);
    float c = dot(ro, ro) - r * r;
    float h = b * b - c;
    if (h < 0.0) return false;
    h = sqrt(h);
    t = -b - h;
    return t > 0.001;
}

void main() {
    vec3 eye = uEyePos;
    vec3 fwd = normalize(-eye);
    vec3 rt = normalize(cross(fwd, vec3(0, 1, 0)));
    vec3 up = cross(rt, fwd);
    float ar = uRes.x / uRes.y;
    vec2 uv = (vUV - 0.5) * 2.0;
    uv.x *= ar;
    vec3 rd = normalize(fwd + uv.x * rt * 0.55 + uv.y * up * 0.55);

    float t;
    if (!hitSphere(eye, rd, 1.0, t)) {
        outColor = vec4(0.05, 0.05, 0.08, 1.0);
        return;
    }

    vec3 P = eye + rd * t;
    vec3 N = normalize(P);
    vec3 V = normalize(eye - P);
    vec3 L = normalize(uLightDir);

    float NdV = abs(dot(N, V));
    float fresnel = 0.04 + 0.96 * pow(1.0 - NdV, 5.0);

    // 三层镜面高光
    vec3 H = normalize(L + V);
    float NdH = max(dot(N, H), 0.0);
    float specCore = pow(NdH, 600.0);   // 窄核心
    float specGlow = pow(NdH, 80.0);    // 中等辉光
    float specWide = pow(NdH, 15.0);    // 宽散射（中心可见）

    // 环境反射
    vec3 R = reflect(-V, N);
    vec3 skyTop     = vec3(0.25, 0.35, 0.55);
    vec3 skyHorizon = vec3(0.35, 0.45, 0.65);
    vec3 ground     = vec3(0.10, 0.12, 0.18);
    float skyT = smoothstep(-0.15, 0.25, R.y);
    vec3 envRefl = mix(ground, mix(skyHorizon, skyTop, skyT), skyT);

    // 玻璃体：淡蓝色
    vec3 glassBody = vec3(0.48, 0.70, 0.92);
    vec3 glassColor = mix(glassBody, envRefl, fresnel);

    // 镜面高光：三层叠加
    vec3 specColor = specCore * vec3(0.95, 0.97, 1.00) * 30000.0;
    specColor += specGlow * vec3(0.90, 0.92, 0.95) * 8.0;
    specColor += specWide * vec3(0.85, 0.87, 0.90) * 0.8;
    glassColor += specColor;

    float alpha = 0.35 + fresnel * 0.50 + specCore * 0.4 + specGlow * 0.15 + specWide * 0.05;
    alpha = clamp(alpha * (1.0 - P0 + 0.2), 0.08, 1.0);

    outColor = vec4(glassColor * uLightColor, alpha);
}
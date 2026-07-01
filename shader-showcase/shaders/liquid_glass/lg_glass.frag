#version 460
// LiquidGlass 核心: 超椭圆 SDF + 折射 + 噪声 + 发光
// 与 BatchRenderer2D.glsl LiquidGlass() 完全一致
// P2 < 1.5 → LiquidGlass squircle; P2 >= 1.5 → 背景直通

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D uInputTex;
layout(std140, binding = 1) uniform Params {
    float P0;       // u_powerFactor (超椭圆指数)
    float P1;       // u_fPower (折射强度曲线)
    float P2;       // 模式: 1.0=玻璃, 2.0=背景直通
    float P3;       // u_noise
    float P4;       // u_glowWeight
    float P5;       // u_glowBias
    vec2 uRes; float uTime; float uFC;
    mat4 m0;        // m0[0]=vec4(u_a,u_b,u_c,u_d), m0[1].xy=(u_glowEdge0,u_glowEdge1)
    mat4 m1;
    vec3 uLightDir; float _p0;
    vec3 uLightColor; float _p1;
    vec3 uEyePos; float _p2;
};

#define M_E 2.718281828459045

float sdSuperellipse(vec2 p, float n, float r) {
    vec2 p_abs = abs(p);
    float num = pow(p_abs.x, n) + pow(p_abs.y, n) - pow(r, n);
    float den = n * sqrt(pow(p_abs.x, 2.0 * n - 2.0) + pow(p_abs.y, 2.0 * n - 2.0)) + 0.00001;
    return num / den;
}

float rand2(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float Glow() {
    return sin(atan(vUV.y * 2.0 - 1.0, vUV.x * 2.0 - 1.0) - 0.5);
}

void main() {
    // 背景直通模式 (P2 >= 1.5)
    if (P2 > 1.5) {
        outColor = texture(uInputTex, vUV);
        return;
    }

    float u_a = m0[0].x;
    float u_b = m0[0].y;
    float u_c = m0[0].z;
    float u_d = m0[0].w;
    float u_glowEdge0 = m0[1].x;
    float u_glowEdge1 = m0[1].y;

    vec2 p = (vUV - 0.5) * 2.0;
    float d = sdSuperellipse(p, P0, 1.0);

    if (d > 0.0)
        discard;

    float dist = -d;
    // f(x) = 1 - u_b * (u_c * e)^(-u_d * x - u_a)
    float refr = 1.0 - u_b * pow(u_c * M_E, -u_d * dist - u_a);
    vec2 sampleP = p * pow(refr, P1);

    vec2 coord = sampleP * 0.5 + 0.5;

    if (max(coord.x, coord.y) > 1.0 || min(coord.x, coord.y) < 0.0) {
        outColor = vec4(1.0, 0.0, 1.0, 1.0);
        return;
    }

    vec4 noise = vec4(vec3(rand2(gl_FragCoord.xy * 1e-3) - 0.5), 0.0);
    vec4 color = texture(uInputTex, coord) + noise * P3;
    float mul = Glow() * P4 * smoothstep(u_glowEdge0, u_glowEdge1, dist) + 1.0 + P5;
    outColor = color * vec4(vec3(mul), 1.0);
}
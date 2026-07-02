#version 460
// LiquidGlass 核心: 超椭圆 SDF + 折射 + 噪声 + 发光
// P2 < 1.5 → LiquidGlass squircle; P2 >= 1.5 → 背景直通

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D uInputTex;
layout(std140, binding = 1) uniform Params {
    float P0; float P1; float P2; float P3; float P4; float P5;
    vec2 uRes; float uTime; float uFC;
    mat4 m0; mat4 m1;
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

float rand2(vec2 co) { return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453); }

float Glow(vec2 sdfCoord) {
    vec2 tc = sdfCoord * 0.5 + 0.5;
    return sin(atan(tc.y, tc.x) - 0.5);
}

void main() {
    if (P2 > 1.5) { outColor = texture(uInputTex, vUV); return; }

    float u_a = m0[0].x, u_b = m0[0].y, u_c = m0[0].z, u_d = m0[0].w;
    float u_glowEdge0 = m0[1].x, u_glowEdge1 = m0[1].y;
    float u_scaleX = m1[0].x, u_scaleY = m1[0].y;

    vec2 p = (vUV - 0.5) * 2.0;
    vec2 ps = p * vec2(u_scaleX, u_scaleY);
    float d = sdSuperellipse(ps, P0, 1.0);

    if (d > 0.0) discard;

    float dist = -d;
    float refr = 1.0 - u_b * pow(u_c * M_E, -u_d * dist - u_a);
    vec2 sampleP = ps * pow(refr, P1);
    vec2 coord = sampleP / vec2(u_scaleX, u_scaleY) * 0.5 + 0.5;

    if (max(coord.x, coord.y) > 1.0 || min(coord.x, coord.y) < 0.0) {
        outColor = vec4(1.0, 0.0, 1.0, 1.0); return;
    }

    vec4 noise = vec4(vec3(rand2(gl_FragCoord.xy * 1e-3) - 0.5), 0.0);
    vec4 color = texture(uInputTex, coord) + noise * P3;
    float mul = Glow(ps) * P4 * smoothstep(u_glowEdge0, u_glowEdge1, dist) + 1.0 + P5;
    outColor = color * vec4(vec3(mul), 1.0);
}
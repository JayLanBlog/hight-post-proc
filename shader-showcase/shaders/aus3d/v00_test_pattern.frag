#version 460
// 程序化城市黄昏场景: 天空+太阳+云层+远山+建筑群+水面反射
// 用于后处理特效(水幕/径向模糊/油画/像素化/高斯模糊)的输入画面
// 细节丰富, 使后处理效果清晰可见
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash(i), hash(i + vec2(1,0)), f.x),
               mix(hash(i + vec2(0,1)), hash(i + vec2(1,1)), f.x), f.y);
}

float fbm(vec2 p) {
    float v = 0.0, a = 0.5;
    for (int i = 0; i < 4; i++) {
        v += a * noise(p);
        p *= 2.0;
        a *= 0.5;
    }
    return v;
}

void main() {
    vec2 uv = vUV;
    vec3 col = vec3(0.0);

    // === 天空渐变 ===
    float skyGrad = uv.y;
    vec3 skyTop = vec3(0.05, 0.1, 0.35);
    vec3 skyMid = vec3(0.85, 0.35, 0.15);
    vec3 skyBot = vec3(0.95, 0.7, 0.4);
    vec3 sky = mix(skyTop, skyMid, smoothstep(0.0, 0.45, skyGrad));
    sky = mix(sky, skyBot, smoothstep(0.45, 0.75, skyGrad));

    // === 太阳 ===
    float sunDist = length(uv - vec2(0.75, 0.35));
    float sunGlow = exp(-sunDist * 12.0);
    float sunCore = smoothstep(0.04, 0.0, sunDist);
    sky += vec3(1.0, 0.8, 0.3) * sunGlow * 0.6;
    sky += vec3(1.0, 0.95, 0.8) * sunCore;

    // === 云层 ===
    vec2 cloudUV = uv * vec2(3.0, 1.5) + vec2(uTime * 0.02, 0.0);
    float cloud1 = fbm(cloudUV + vec2(0.0, 0.0));
    float cloud2 = fbm(cloudUV + vec2(1.5, 0.3));
    float cloud = smoothstep(0.4, 0.7, cloud1 * 0.6 + cloud2 * 0.4);
    sky = mix(sky, vec3(0.95, 0.85, 0.7), cloud * 0.25 * smoothstep(0.15, 0.5, uv.y));

    // === 远山 ===
    if (uv.y < 0.55) {
        float mountain = 0.0;
        for (int i = 0; i < 5; i++) {
            float fi = float(i);
            float h = fbm(vec2(uv.x * 2.5 + fi * 0.7, 0.0)) * 0.12 + 0.38;
            mountain = max(mountain, smoothstep(h - 0.01, h, uv.y) * (0.5 + fi * 0.1));
        }
        vec3 mtColor = mix(vec3(0.2, 0.25, 0.3), vec3(0.3, 0.35, 0.4), mountain);
        float mtFog = smoothstep(0.35, 0.55, uv.y);
        sky = mix(sky, mtColor, mountain * (1.0 - mtFog * 0.5));
    }

    // === 地面 ===
    float groundY = 0.4;
    vec3 ground = vec3(0.25, 0.3, 0.2);

    // === 建筑 ===
    if (uv.y < groundY && uv.y > 0.15) {
        float building = 0.0;
        vec3 bldColor = vec3(0.0);

        for (int i = 0; i < 20; i++) {
            float fi = float(i);
            float bx = hash(vec2(fi, 0.0));
            float bw = 0.02 + hash(vec2(fi, 1.0)) * 0.06;
            float bh = 0.08 + hash(vec2(fi, 2.0)) * 0.22;
            float by = groundY - bh;

            float inX = smoothstep(bx - bw, bx, uv.x) - smoothstep(bx + bw, bx + bw + 0.005, uv.x);
            float inY = smoothstep(by, by + 0.005, uv.y) - smoothstep(groundY, groundY + 0.005, uv.y);

            if (inX > 0.0 && inY > 0.0) {
                building = 1.0;
                // 建筑颜色：深灰到棕
                vec3 bld = mix(vec3(0.15, 0.15, 0.18), vec3(0.35, 0.3, 0.25), hash(vec2(fi, 3.0)));
                // 窗户灯光
                float winY = fract(uv.y * 40.0 + fi * 0.7);
                float winX = fract(uv.x * 25.0 + fi * 1.3);
                float win = step(0.7, winY) * step(0.15, winX) * step(winX, 0.85);
                float winOn = step(0.5, hash(vec2(fi, floor(uv.y * 40.0) + floor(uv.x * 25.0) * 0.1)));
                bldColor = mix(bld, vec3(0.9, 0.75, 0.4), win * winOn * 0.7);
            }
        }

        if (building > 0.0) {
            col = bldColor;
            outColor = vec4(col, 1.0);
            return;
        }
        col = ground;
    }

    // === 水面 ===
    if (uv.y < 0.15) {
        float waterNoise = fbm(uv * vec2(8.0, 4.0) + uTime * 0.03) * 0.1;
        vec3 water = mix(vec3(0.05, 0.1, 0.25), vec3(0.1, 0.2, 0.4), uv.y * 5.0);
        // 水面反射天空
        vec3 reflectSky = sky * 0.6;
        water = mix(water, reflectSky, 0.4);
        // 波纹高光
        water += vec3(0.3, 0.4, 0.5) * waterNoise * 0.5;
        // 太阳倒影
        float sunReflect = exp(-abs(uv.y - 0.05) * 20.0) * exp(-abs(uv.x - 0.75) * 8.0);
        water += vec3(1.0, 0.7, 0.3) * sunReflect * 0.3;
        col = water;
        outColor = vec4(col, 1.0);
        return;
    }

    col = sky;
    outColor = vec4(col, 1.0);
}
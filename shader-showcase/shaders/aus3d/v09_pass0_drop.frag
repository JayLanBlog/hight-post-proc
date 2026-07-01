#version 460
// 水幕特效: 全屏后处理 - 水滴纹理UV扭曲
// 与 Awesome-Unity-Shader Volume 09 ScreenWaterDropEffect 完全一致
// P0=_DropSpeed(0-10,默认3.6) P1=_Distortion(5-64,默认8.0) P2=_SizeX(0-7,默认1.0) P3=_SizeY(0-7,默认0.5)
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;  // 球体预渲染画面(对应_MainTex)
layout(binding=2) uniform sampler2D uAuxTex;     // 水滴纹理(对应_ScreenWaterDropTex)
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

void main() {
    float dropSpeed = P0;    // _DropSpeed (0-10, default 3.6)
    float distortion = P1;   // _Distortion (5-64, default 8.0)
    float sizeX = P2;        // _SizeX (0-7, default 1.0)
    float sizeY = P3;        // _SizeY (0-7, default 0.5)

    // 三层水滴纹理采样 (与Unity版本完全一致, fract()适配CLAMP采样器)
    vec3 rainTex1 = texture(uAuxTex,
        fract(vec2(vUV.x * 1.15 * sizeX, vUV.y * sizeY * 1.1 + uTime * dropSpeed * 0.15))).rgb / distortion;
    vec3 rainTex2 = texture(uAuxTex,
        fract(vec2(vUV.x * 1.25 * sizeX - 0.1, vUV.y * sizeY * 1.2 + uTime * dropSpeed * 0.2))).rgb / distortion;
    vec3 rainTex3 = texture(uAuxTex,
        fract(vec2(vUV.x * sizeX * 0.9, vUV.y * sizeY * 1.25 + uTime * dropSpeed * 0.032))).rgb / distortion;

    // 计算最终UV偏移 (与Unity版本完全一致)
    vec2 finalUV = vUV - (rainTex1.xy - rainTex2.xy - rainTex3.xy) / 3.0;

    // 扭曲采样
    vec3 finalColor = texture(uInputTex, finalUV).rgb;

    outColor = vec4(finalColor, 1.0);
}
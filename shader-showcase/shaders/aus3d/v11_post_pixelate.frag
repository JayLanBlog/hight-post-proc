#version 460
// 像素化：UV坐标ceil取整 → 马赛克效果
// Reference: Volume 11 PixelEffect, _Params.x = pixelNum, _Params.y = ratio
//            coordX = PixelSize * ceil(uv.x / PixelSize)
//            coordY = (ratio * PixelSize) * ceil(uv.y / PixelSize / ratio)
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

void main() {
    float scale = P0 * 80.0;  // P0=1.0 → 80 pixels/row (matches reference _Params.x=80)
    float ratio = P1;          // P1=1.0 → ratio=1 (matches reference _Params.y=1)
    float pixelSize = 1.0 / scale;
    float px = pixelSize * ceil(vUV.x / pixelSize);
    float py = (ratio * pixelSize) * ceil(vUV.y / pixelSize / ratio);
    outColor = texture(uInputTex, vec2(px, py));
}
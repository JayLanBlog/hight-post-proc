#version 460
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};

float R(vec2 s){return fract(sin(dot(s,vec2(127.1,311.7)))*43758.5453);}
void main(){
    float sh=(R(vec2(uTime,2.0))-0.5)*uParamFloat0*0.25;
    vec2 uv=vUV;if(0.0>0.5)uv=fract(vec2(vUV.x,vUV.y+sh));else uv=fract(vec2(vUV.x+sh,vUV.y));
    fC=texture(uInputTex,uv);
}

#version 460
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};

void main(){
    float jt=uTime*uParamFloat0*9.8;
    float jx=mix(vUV.x,fract(vUV.x+jt),uParamFloat0);
    float jy=mix(vUV.y,fract(vUV.y+jt),uParamFloat0);
    vec2 uv=0.0>0.5?fract(vec2(vUV.x,jy)):fract(vec2(jx,vUV.y));
    fC=texture(uInputTex,uv);
}

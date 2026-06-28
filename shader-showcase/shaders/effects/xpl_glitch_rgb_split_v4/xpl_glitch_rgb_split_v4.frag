#version 460
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};

float R(vec2 s){return fract(sin(dot(s,vec2(12.9898,78.233)))*43758.5453);}
void main(){
    float tX=floor(uTime*uParamFloat1);
    float sa=uParamFloat0*0.1*R(vec2(tX,2.0));
    float r=texture(uInputTex,vec2(vUV.x+sa,vUV.y)).r;
    float g=texture(uInputTex,vUV).g;
    float b=texture(uInputTex,vec2(vUV.x-sa,vUV.y)).b;
    fC=vec4(r,g,b,1.0);
}

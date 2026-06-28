#version 460
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};

float R(vec2 s){return fract(sin(dot(s,vec2(12.9898,78.233)))*43758.5453);}
void main(){
    float t=clamp(1.0-uParamFloat0*1.2,0.0,1.0);
    float a=0.005+pow(uParamFloat0,3.0)*0.1;
    float s=0.5+0.5*cos(uTime*uParamFloat1);
    float j=R(vec2(vUV.y,uTime))*2.0-1.0;
    j*=step(t,abs(j))*a*s;
    fC=texture(uInputTex,fract(vUV+vec2(j,0.0)));
}

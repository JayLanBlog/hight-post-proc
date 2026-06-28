#version 460
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};

float R(vec2 s){return fract(sin(dot(s,vec2(12.9898,78.233)))*43758.5453);}
void main(){
    float bx=R(floor(vUV*uParamFloat1));
    float by=R(floor(vUV*uParamFloat1+vec2(0,1)));
    float dn=pow(bx,8.0)*pow(bx,3.0);
    float sn=pow(R(vec2(7.2341)),17.0);
    float ox=dn-sn*uParamFloat2;float oy=dn-sn*uParamFloat3;
    vec2 off=vec2(ox*0.05*R(vec2(13.0)),oy*0.05*R(vec2(7.0)));
    float r=texture(uInputTex,vUV).r;
    float g=texture(uInputTex,vUV+off).g;
    float b=texture(uInputTex,vUV-off).b;
    fC=vec4(r,g,b,1.0);
}

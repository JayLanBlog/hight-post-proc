#version 460
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};

float R(vec2 s){return fract(sin(dot(s,vec2(12.9898,78.233)))*43758.5453);}
vec4 N(vec2 p){return vec4(R(p),R(p+vec2(0.33,0.67)),R(p+vec2(0.67,0.33)),R(p+vec2(0.5,0.5)));}
void main(){
    vec2 nu=vec2(uParamFloat1*uTime,2.0*uParamFloat1*uTime/25.0);
    vec4 sa=vec4(pow(N(nu).x,8.0),pow(N(nu+vec2(0.1,0.2)).y,8.0),pow(N(nu+vec2(0.3,0.4)).z,8.0),1.0)*uParamFloat0;
    sa*=2.0*sa.w-1.0;
    float r=texture(uInputTex,vUV+vec2(sa.x,-sa.y)).r;
    float g=texture(uInputTex,vUV+vec2(sa.y,-sa.z)).g;
    float b=texture(uInputTex,vUV+vec2(sa.z,-sa.x)).b;
    fC=vec4(r,g,b,1.0);
}

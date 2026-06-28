#version 460
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};

float R(vec2 s){return fract(sin(dot(s,vec2(12.9898,78.233)))*43758.5453);}
void main(){
    vec4 sc=texture(uInputTex,vUV);vec4 nc=sc;
    float lum=dot(nc.rgb,vec3(0.22,0.707,0.071));
    if(R(vec2(uTime*uParamFloat0))>uParamFloat2)nc=vec4(vec3(lum),sc.a);
    float nx=R(vec2(uTime*uParamFloat0+vUV.x/-213.0,uTime*uParamFloat0+vUV.y/5.53));
    float ny=R(vec2(uTime*uParamFloat0-vUV.x/213.0,uTime*uParamFloat0-vUV.y/-5.53));
    float nz=R(vec2(uTime*uParamFloat0+vUV.x/213.0,uTime*uParamFloat0+vUV.y/5.53));
    nc.rgb+=0.25*vec3(nx,ny,nz)-0.125;nc=mix(sc,nc,uParamFloat1);fC=nc;
}

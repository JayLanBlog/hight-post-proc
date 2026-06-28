#version 460
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};

float R(vec2 s){return fract(sin(dot(s,vec2(12.9898,78.233)))*43758.5453);}
void main(){
    vec4 sn=vec4(R(vUV*200.0+vec2(uTime*0.1)),R(vUV*200.0+vec2(0.3,0.7)+uTime*0.13),
                 R(vUV*200.0+vec2(0.7,0.3)+uTime*0.17),R(vUV*200.0+vec2(0.5,0.5)+uTime*0.11));
    float t=1.001-uParamFloat0*1.001;
    float us=step(t,pow(abs(sn.x),3.0));
    vec2 uv=fract(vUV+sn.yz*us);
    vec4 src=texture(uInputTex,uv);
    vec3 sc=vec3(uParamFloat2,uParamFloat3,uParamFloat4);
    float si=step(t,pow(abs(sn.w),3.0))*uParamFloat1;
    vec3 col=mix(src.rgb,sc,si);
    fC=vec4(col,src.a);
}

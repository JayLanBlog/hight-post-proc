#version 460
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0; // uParamFloat0
    float uParamFloat1; // uParamFloat1
    float uParamFloat2; // uParamFloat2
    float uParamFloat3; // uParamFloat3
    float uParamFloat4; // uParamFloat4
    float uParamFloat5; // uParamFloat5
    vec2 uResolution;
    float uTime;
    float uFrameCount;
};;
float R(vec2 s){return fract(sin(dot(s,vec2(12.9898,78.233)))*43758.5453);}
float T(float x,float L){return floor(x*L)/L;}
vec2 T2(vec2 x,vec2 L){return floor(x*L)/L;}
vec3 rgb2yuv(vec3 c){return vec3(dot(c,vec3(0.299,0.587,0.114)),dot(c,vec3(-0.14713,-0.28886,0.436)),dot(c,vec3(0.615,-0.51499,-0.10001)));}
vec3 yuv2rgb(vec3 c){return vec3(dot(c,vec3(1.0,0.0,1.13983)),dot(c,vec3(1.0,-0.39465,-0.58060)),dot(c,vec3(1.0,2.03211,0.0)));}
void main(){
    float tX=uTime*uParamFloat1*0.2;vec2 uv=vUV;
    float st=0.5+0.5*cos(tX*uParamFloat0);tX*=st;
    float tT=T(tX,4.0);
    vec2 ac=0.0>0.5?uv.xx:uv.yy;
    float ut=R(T2(ac,vec2(8.0))+100.0*tT);
    float ur=6.0*T(tX,24.0*ut);
    float lr=0.5*R(T2(ac+vec2(ur),vec2(8.0/uParamFloat4)))+0.5*R(T2(ac+vec2(ur),vec2(7.0)));
    lr=lr*2.0-1.0;lr=sign(lr)*clamp((abs(lr)-uParamFloat2)/0.4,0.0,1.0);lr=mix(0.0,lr,uParamFloat3);
    vec2 ubl=uv;if(0.0>0.5)ubl=clamp(ubl+vec2(0.0,0.1*lr),0.0,1.0);else ubl=clamp(ubl+vec2(0.1*lr,0.0),0.0,1.0);
    vec4 blC=texture(uInputTex,abs(ubl));
    vec3 yuv=rgb2yuv(blC.rgb);
    yuv.y/=1.0-3.0*abs(lr)*clamp(0.5-lr,0.0,1.0);
    yuv.z+=0.125*lr*clamp(lr-0.5,0.0,1.0);
    vec3 rgb=yuv2rgb(yuv);
    vec4 sc=texture(uInputTex,uv);
    fC=mix(sc,vec4(rgb,blC.a),uParamFloat5);
}

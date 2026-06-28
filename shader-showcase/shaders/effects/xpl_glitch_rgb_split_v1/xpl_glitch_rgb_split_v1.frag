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
void main(){
    vec2 uv=vUV;float t=uTime*6.0*uParamFloat2;
    float sa=(1.0+sin(t))*0.5;sa*=1.0+sin(t*2.0)*0.5;
    sa=pow(sa,3.0)*0.05;
    float d=length(uv-vec2(0.5));
    sa*=uParamFloat0*uParamFloat1;sa*=mix(1.0,d,uParamFloat5);
    vec3 sc=texture(uInputTex,uv).rgb;
    if(0.0<0.5){
        float r=texture(uInputTex,vec2(uv.x+sa*uParamFloat3,uv.y)).r;
        float b=texture(uInputTex,vec2(uv.x-sa*uParamFloat4,uv.y)).b;
        fC=vec4(mix(sc,vec3(r,sc.g,b),uParamFloat0),1.0);
    }else if(0.0<1.5){
        float r=texture(uInputTex,vec2(uv.x,uv.y+sa*uParamFloat3)).r;
        float b=texture(uInputTex,vec2(uv.x,uv.y-sa*uParamFloat4)).b;
        fC=vec4(mix(sc,vec3(r,sc.g,b),uParamFloat0),1.0);
    }else{
        float r=texture(uInputTex,vec2(uv.x+sa*uParamFloat3,uv.y+sa*uParamFloat3)).r;
        float b=texture(uInputTex,vec2(uv.x-sa*uParamFloat4,uv.y-sa*uParamFloat4)).b;
        fC=vec4(mix(sc,vec3(r,sc.g,b),uParamFloat0),1.0);
    }
}

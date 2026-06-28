#version 460
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};

void main(){
    float sa=(1.0+sin(uTime*6.0))*0.5;
    sa*=1.0+sin(uTime*16.0)*0.5;sa*=1.0+sin(uTime*19.0)*0.5;
    sa*=1.0+sin(uTime*27.0)*0.5;sa=pow(sa,uParamFloat1)*0.05*uParamFloat0;
    vec3 c=vec3(texture(uInputTex,vec2(vUV.x+sa,vUV.y)).r,texture(uInputTex,vUV).g,texture(uInputTex,vec2(vUV.x-sa,vUV.y)).b);
    c*=1.0-sa*0.5;fC=vec4(c,1.0);
}

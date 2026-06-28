#version 460
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};

void main(){
    vec2 uv=vUV;float s=0.5+0.5*cos(uTime*uParamFloat3);float ps=1.0/1920.0;
    bool j=mod(uv.y*uParamFloat0,2.0)<1.0;
    if(j){float a=ps*cos(uTime*uParamFloat2*100.0)*uParamFloat1*s;uv.x+=a;}
    fC=texture(uInputTex,uv);
}

#version 460
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};

void main(){
    float a=uParamFloat1*0.001;float s=0.5+0.5*cos(uTime*uParamFloat0);a*=s;vec2 uv=vUV;
    float r,g,b;
    if(0.0<0.5){
        r=texture(uInputTex,vec2(uv.x+sin(uTime*uParamFloat2*0.2)*a,uv.y)).r;
        g=texture(uInputTex,vec2(uv.x+sin(uTime*uParamFloat2*0.1)*a,uv.y)).g;
        b=texture(uInputTex,vec2(uv.x+sin(uTime*uParamFloat2*0.05)*a,uv.y)).b;
    }else if(0.0<1.5){
        r=texture(uInputTex,vec2(uv.x,uv.y+sin(uTime*uParamFloat2*0.2)*a)).r;
        g=texture(uInputTex,vec2(uv.x,uv.y+sin(uTime*uParamFloat2*0.1)*a)).g;
        b=texture(uInputTex,vec2(uv.x,uv.y-sin(uTime*uParamFloat2*0.05)*a)).b;
    }else{
        r=texture(uInputTex,vec2(uv.x+sin(uTime*uParamFloat2*0.2)*a,uv.y+sin(uTime*uParamFloat2*0.2)*a)).r;
        g=texture(uInputTex,vec2(uv.x+sin(uTime*uParamFloat2*0.1)*a,uv.y+sin(uTime*uParamFloat2*0.1)*a)).g;
        b=texture(uInputTex,vec2(uv.x-sin(uTime*uParamFloat2*0.05)*a,uv.y-sin(uTime*uParamFloat2*0.05)*a)).b;
    }
    fC=vec4(r,g,b,1.0);
}

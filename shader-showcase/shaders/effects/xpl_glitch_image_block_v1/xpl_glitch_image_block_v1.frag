#version 460
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
float R(vec2 s){return fract(sin(dot(s*floor(uTime*uParamFloat0*30.0),vec2(127.1,311.7)))*43758.5453123);}
float Rf(float s){return R(vec2(s,1.0));}
void main(){
    vec2 b1=floor(vUV*vec2(uParamFloat2,uParamFloat3));vec2 b2=floor(vUV*vec2(uParamFloat4,uParamFloat5));
    float ln1=pow(R(b1),8.0);float ln2=pow(R(b2),4.0);
    float rn=pow(Rf(5.1379),7.1)*0.5;
    float ln=ln1*ln2*uParamFloat1-rn;
    float r=texture(uInputTex,vUV).r;
    float g=texture(uInputTex,vUV+vec2(ln*0.05*Rf(7.0),0.0)).g;
    float b=texture(uInputTex,vUV-vec2(ln*0.05*Rf(23.0),0.0)).b;
    vec4 res=vec4(r,g,b,1.0);vec4 orig=texture(uInputTex,vUV);
    fC=mix(orig,res,1.0);
}

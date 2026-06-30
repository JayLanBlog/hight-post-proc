#version 460
// Vol.05-22 Glass v2: cubemap reflection with specular highlight on sphere
// 天空渐变反射 + 菲涅尔 + 三层镜面高光 + 半透明
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(binding=2) uniform sampler2D uAuxTex; // reserved for future cubemap
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

bool hit(vec3 ro,vec3 rd,float r,out float t){
    float b=dot(ro,rd),c=dot(ro,ro)-r*r,h=b*b-c;
    if(h<0.0)return false;h=sqrt(h);t=-b-h;return t>0.001;
}

void main(){
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float a=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=a;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float t;if(!hit(eye,rd,1.0,t)){outColor=vec4(0.02,0.02,0.04,1);return;}
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 V=normalize(eye-P);
    vec3 R=reflect(-V,N);

    // 程序化天空立方体贴图反射
    float skyGrad=clamp(R.y*0.5+0.5,0.0,1.0);
    vec3 skyTop=vec3(0.3,0.6,1.0);
    vec3 skyHorizon=vec3(0.8,0.9,1.0);
    vec3 ground=vec3(0.15,0.22,0.35);
    vec3 refl=mix(ground,mix(skyHorizon,skyTop,skyGrad),skyGrad);
    float fresnel=0.04 + 0.96 * pow(1.0 - abs(dot(N,V)), 5.0);

    vec3 L=normalize(uLightDir);
    float ndl=dot(N,L)*0.5+0.5;
    vec3 diffuse=vec3(P0,P1,P2)*ndl*0.3;

    // 三层镜面高光
    vec3 H=normalize(L + V);
    float NdH=max(dot(N,H),0.0);
    float specCore=pow(NdH,80.0);
    float specGlow=pow(NdH,20.0);
    float specWide=pow(NdH,8.0);
    vec3 specColor=specCore * vec3(0.95,0.95,1.0) * 5.0;
    specColor+=specGlow * vec3(0.90,0.90,0.95) * 5.0;
    specColor+=specWide * vec3(0.85,0.85,0.90) * 2.0;

    vec3 col=mix(diffuse,refl,fresnel*0.8);
    col += specColor;
    outColor=vec4(col*uLightColor,0.55);
}
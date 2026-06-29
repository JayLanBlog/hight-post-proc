#version 460
// CarPaint — blue metallic with distinct specular + fresnel
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
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
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 V=normalize(eye-P);vec3 L_=normalize(uLightDir);
    vec3 H=normalize(L_+V);
    float specPow=P0*80.0+16.0;
    float spec=pow(max(dot(N,H),0.0),specPow);
    float fresnel=pow(1.0-abs(dot(N,V)),3.0);
    float facing=dot(N,normalize(-eye));
    // Brighter blue base -- deep blue center, lighter blue at edges
    vec3 base=mix(vec3(0.05,0.10,0.30),vec3(0.10,0.22,0.55),facing);
    // Warm specular highlight
    vec3 col=base+vec3(1.0,0.9,0.7)*spec*1.0+vec3(0.3,0.5,1.0)*fresnel*0.6;
    // Procedural flake sparkle
    vec2 mc=vec2(dot(N,rt)*0.5+0.5,dot(N,up)*0.5+0.5);
    float flake=texture(uInputTex,mc*10.0+uTime*0.03).r;
    col+=flake*0.08;
    // Subtle diffuse from light
    float ndl=dot(N,L_)*0.5+0.5;
    col*=0.6+0.4*ndl;
    outColor=vec4(col,1);
}

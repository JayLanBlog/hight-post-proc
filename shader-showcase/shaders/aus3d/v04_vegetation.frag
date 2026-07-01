#version 460
// Vol.04-17 Vegetation: 植被渲染 — 双Pass模拟
// Reference: Pass1: AlphaTest Greater[_Cutoff], ZWrite On, combine texture * primary
//            Pass2: ZWrite Off, ZTest Less, AlphaTest LEqual[_Cutoff], Blend SrcAlpha OneMinusSrcAlpha
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
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 L=normalize(uLightDir);
    vec2 suv=vec2(atan(P.z,P.x)*0.1591549+0.5,acos(clamp(P.y,-1.0,1.0))*0.3183099);
    vec4 tex=texture(uInputTex,suv);
    // Reference: combine texture * primary, Cull Off → abs(NdotL)
    float ndl=abs(dot(N,L));
    vec3 ambient=vec3(0.15);
    vec3 diffuse=uLightColor*ndl;
    vec3 primary=ambient+diffuse;
    vec3 color=tex.rgb*primary;
    float cutoff=P0;
    if(tex.a>=cutoff){
        // Pass1: AlphaTest Greater[_Cutoff], opaque
        outColor=vec4(color,1.0);
    }else{
        // Pass2: AlphaTest LEqual[_Cutoff], Blend SrcAlpha OneMinusSrcAlpha
        // Reference: alpha from texture directly
        outColor=vec4(color,tex.a);
    }
}
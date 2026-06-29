#version 460
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
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
    float ndl=dot(N,L);
    int lv=int(max(P0,2.0));float toon=floor(max(ndl*0.5+0.5,0.0)*float(lv))/float(lv);
    vec3 a0=vec3(0.08,0.06,0.15);vec3 a1=vec3(0.25,0.15,0.10);
    vec3 a2=vec3(0.50,0.35,0.25);vec3 a3=vec3(0.75,0.60,0.40);vec3 a4=vec3(1.0,0.90,0.70);
    vec3 col=toon<0.2?a0:toon<0.4?a1:toon<0.6?a2:toon<0.8?a3:a4;
    outColor=vec4(col*uLightColor,1);
}

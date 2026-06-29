#version 460
// Vol.07 卡通渐变
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
    vec3 rd=normalize(fwd+uv.x*rt*0.7+uv.y*up*0.7);
    float t;if(!hit(eye,rd,1.0,t)){outColor=vec4(0.05,0.05,0.08,1);return;}
    vec3 N=normalize(eye+rd*t),L=normalize(uLightDir);
    float ndl=dot(N,L)*0.5+0.5;
    float lv=max(P0,2.0);
    float toon=floor(ndl*lv)/lv;
    vec3 col=vec3(1.0,0.85,0.7)*uLightColor*toon;
    outColor=vec4(col,1);
}

#version 460
// Vol.05 ProgrammableShader: diffuse+specular+ambient (vertex+fragment CGPROGRAM equivalent)
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
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 V=normalize(eye-P);
    vec3 L=normalize(uLightDir);
    // Diffuse (Lambert)
    float ndl=max(dot(N,L),0.0);
    vec3 diffuse=uLightColor*vec3(P0,P1,P2)*ndl;
    // Specular (Blinn-Phong)
    vec3 H=normalize(L+V);
    float spec=pow(max(dot(N,H),0.0),P3);
    vec3 specular=uLightColor*vec3(0.8,0.8,0.8)*spec*P4;
    // Ambient
    vec3 ambient=vec3(P0,P1,P2)*0.15;
    vec3 col=diffuse+specular+ambient;
    outColor=vec4(col,1);
}
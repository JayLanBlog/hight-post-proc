#version 460
// Vol.02-6 光照材质完备正式版: 顶点光照 + 程序化棋盘格纹理 + DOUBLE
// Reference: Volume 02 6.光照材质完备正式版Shader — Combine texture * primary DOUBLE
// SeparateSpecular On: specular pass added after texture combine
// Shininess=0.7 → specPower=128 (Unity fixed-function mapping: 2^(Shininess*10))
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
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 L=normalize(uLightDir);vec3 V=normalize(eye-P);
    float phi=atan(N.z,N.x);float theta=acos(N.y);
    float tiles=8.0*P0;
    float isBright=fract(floor(phi*tiles/3.14159)+floor(theta*tiles/3.14159)*0.5+0.51)>0.5?1.0:0.0;
    vec3 brightColor=vec3(P1,P2,0.4);
    vec3 darkColor=vec3(0.2,0.2,0.5);
    vec3 texColor=mix(darkColor,brightColor,isBright);
    // Reference: Combine texture * primary DOUBLE, SeparateSpecular On
    float ndl=max(dot(N,L),0.0);
    vec3 ambient=vec3(0.1);
    vec3 diffuse=uLightColor*ndl;
    vec3 primary=ambient+diffuse;
    vec3 color=texColor*primary*2.0; // DOUBLE
    // SeparateSpecular: Shininess=0.7 → specPower=128
    vec3 R=reflect(-L,N);
    float spec=pow(max(dot(R,V),0.0),128.0);
    vec3 specular=uLightColor*spec;
    color+=specular;
    outColor=vec4(color,1.0);
}
#version 460
// Vol.12-7 Diffuse(Lambert) with Texture: procedural checkerboard on sphere
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
    // Procedural checkerboard using spherical coordinates
    float phi=atan(N.z,N.x);float theta=acos(N.y);
    float tiles=8.0*P0;
    float check=fract(floor(phi*tiles/3.14159)+floor(theta*tiles/3.14159)*0.5)>0.5?1.0:0.3;
    vec3 texColor=vec3(P1,P2,0.3)*check+vec3(0.3,0.3,P2)*(1.0-check);
    float ndl=max(dot(N,L),0.0);
    vec3 diffuse=uLightColor*texColor*ndl;
    vec3 ambient=texColor*0.15;
    outColor=vec4(diffuse+ambient,1);
}
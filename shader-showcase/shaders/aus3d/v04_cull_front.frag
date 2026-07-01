#version 460
// Vol.04-12 CullFront: 渲染物体背面，顶点光照
// Reference: Volume 04 1.用剔除操作渲染对象背面
// Cull Front, Material Emission(0.3,0.3,0.3,0.3) Diffuse(1,1,1,1), Lighting On
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};
bool hit(vec3 ro,vec3 rd,float r,out float t){
    float b=dot(ro,rd),c=dot(ro,ro)-r*r,h=b*b-c;
    if(h<0.0)return false;h=sqrt(h);t=-b-h;return t>0.001;
}
bool hitSecond(vec3 ro,vec3 rd,float r,out float t){
    float b=dot(ro,rd),c=dot(ro,ro)-r*r,h=b*b-c;
    if(h<0.0)return false;h=sqrt(h);t=-b+h;return t>0.001;
}
void main(){
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float a=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=a;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float t;if(!hitSecond(eye,rd,1.0,t)){outColor=vec4(0.02,0.02,0.04,1);return;}
    vec3 P=eye+rd*t;vec3 N=-normalize(P); // flip normal for back face
    vec3 L=normalize(uLightDir);
    // Reference: Material Emission(0.3,0.3,0.3,0.3) Diffuse(1,1,1,1), Lighting On
    float ndl=max(dot(N,L),0.0); // Lambert
    vec3 ambient=vec3(0.15);
    vec3 diffuse=vec3(1.0)*ndl; // Diffuse(1,1,1,1)
    vec3 emission=vec3(0.3); // Emission(0.3,0.3,0.3,0.3)
    vec3 col=ambient+diffuse*uLightColor+emission;
    outColor=vec4(col,1.0);
}
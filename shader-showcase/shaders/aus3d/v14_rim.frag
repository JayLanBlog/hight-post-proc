#version 460
// Vol.14 Rim glow — soft wide edge light
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
    float t;if(!hit(eye,rd,1.0,t)){outColor=vec4(0.02,0.02,0.04,1);return;}
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 V=normalize(eye-P);
    float rim=1.0-abs(dot(N,V));
    // Use P0 as glow power, but make it wider with sqrt for softer falloff
    float glow=pow(max(rim,0.001),P0);
    // Also add a softer inner glow
    float inner=pow(max(rim,0.001),P0*0.4);
    vec3 col=mix(vec3(P1,P2,P3)*inner*0.4,vec3(P1,P2,P3)*glow,0.7);
    // Bright falloff at silhouette
    col+=vec3(P1,P2,P3)*pow(rim,2.0)*0.5;
    outColor=vec4(col,1);
}

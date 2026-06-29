#version 460
// Vol.02 纯色
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uResolution; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

bool hit(vec3 ro, vec3 rd, float r, out float t) {
    float b=dot(ro,rd),c=dot(ro,ro)-r*r,h=b*b-c;
    if(h<0.0)return false; h=sqrt(h);
    t=-b-h; return t>0.001;
}
void main() {
    vec3 eye=uEyePos, fwd=normalize(-eye), rt=normalize(cross(fwd,vec3(0,1,0))), up=cross(rt,fwd);
    float a=uResolution.x/uResolution.y;
    vec3 rd=normalize(fwd+((vUV-0.5)*2.0).x*a*rt*0.7+((vUV-0.5)*2.0).y*up*0.7);
    float t;
    if(!hit(eye,rd,1.0,t)) { outColor=vec4(0.05,0.05,0.08,1); return; }
    vec3 P=eye+rd*t, N=normalize(P);
    vec3 L=normalize(uLightDir);
    float NdotL=max(dot(N,L),0.0);
    vec3 col=vec3(P0,P1,P2)*NdotL*uLightColor;
    outColor=vec4(col,1);
}

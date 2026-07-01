#version 460
// Vol.13-3 TwoSideColorChangeAlpha: 双面双色透明立方体, 无光照 (与项目11完全一致)
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};
bool hitCube(vec3 ro,vec3 rd,float r,out float t,out vec3 n){
    vec3 m=1.0/rd;vec3 n2=m*ro;vec3 k=abs(m)*r;
    vec3 t1=-n2-k;vec3 t2=-n2+k;
    float tN=max(max(t1.x,t1.y),t1.z);
    float tF=min(min(t2.x,t2.y),t2.z);
    if(tN>tF||tF<0.0)return false;
    t=tN>0.0?tN:tF;
    n=-sign(rd)*step(t1,t1.yzx)*step(t1,t1.zxy);
    return true;
}
void main(){
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float ar=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=ar;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float t;vec3 N;
    if(!hitCube(eye,rd,0.4,t,N)){outColor=vec4(0.02,0.02,0.04,1);return;}
    vec3 P=eye+rd*t;vec3 V=normalize(eye-P);
    bool front=dot(N,V)>0.0;
    vec3 col=front?vec3(0.9,0.1,0.1):vec3(0.1,0.3,0.9);
    float alpha=P0;
    outColor=vec4(col,alpha);
}
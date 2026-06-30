#version 460
// Vol.12 RGB Cube — same FOV as sphere
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};
bool hitCube(vec3 ro,vec3 rd,out float t,out vec3 P,out vec3 N){
    vec3 t1=(vec3(-0.4)-ro)/rd; vec3 t2=(vec3(0.4)-ro)/rd;
    vec3 tn=min(t1,t2); vec3 tf=max(t1,t2);
    float ne=max(max(tn.x,tn.y),tn.z);float fa=min(min(tf.x,tf.y),tf.z);
    if(ne>fa||fa<0.001)return false;t=ne>0.001?ne:fa;P=ro+rd*t;
    vec3 ap=abs(P);
    if(ap.x>=ap.y&&ap.x>=ap.z)N=vec3(sign(P.x),0,0);
    else if(ap.y>=ap.x&&ap.y>=ap.z)N=vec3(0,sign(P.y),0);
    else N=vec3(0,0,sign(P.z));return true;
}
void main(){
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float ar=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=ar;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float t;vec3 P,N;
    if(!hitCube(eye,rd,t,P,N)){outColor=vec4(0.02,0.02,0.04,1);return;}
    vec3 L=normalize(uLightDir);
    vec3 col;
    if(abs(N.x)>0.5)col=N.x>0.0?vec3(1,0.2,0.1):vec3(0.1,0.8,0.2);
    else if(abs(N.y)>0.5)col=N.y>0.0?vec3(0.2,0.3,1):vec3(0.9,0.8,0.1);
    else col=N.z>0.0?vec3(0.1,0.9,0.9):vec3(0.9,0.1,0.9);
    float ndl=dot(N,L)*0.5+0.5;col*=0.25+0.75*ndl;
    outColor=vec4(col*uLightColor,1);
}

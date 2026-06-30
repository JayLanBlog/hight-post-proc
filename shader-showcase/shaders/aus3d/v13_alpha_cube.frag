#version 460
// Vol.13 Alpha Cube — glass-like transparency: front+back faces, fresnel, specular
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};
bool hitCube(vec3 ro,vec3 rd,out float tn,out float tf){
    vec3 t1=(vec3(-0.4)-ro)/rd; vec3 t2=(vec3(0.4)-ro)/rd;
    vec3 tnv=min(t1,t2); vec3 tfv=max(t1,t2);
    tn=max(max(tnv.x,tnv.y),tnv.z);
    tf=min(min(tfv.x,tfv.y),tfv.z);
    return tf>0.001&&tn<tf;
}
vec3 cubeNormal(vec3 P){
    vec3 ap=abs(P);
    if(ap.x>=ap.y&&ap.x>=ap.z)return vec3(sign(P.x),0,0);
    if(ap.y>=ap.x&&ap.y>=ap.z)return vec3(0,sign(P.y),0);
    return vec3(0,0,sign(P.z));
}
void main(){
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float ar=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=ar;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float tn,tf;
    if(!hitCube(eye,rd,tn,tf)){outColor=vec4(0.02,0.02,0.04,1);return;}
    vec3 L=normalize(uLightDir);
    float alpha=clamp(1.0-P0,0.05,1.0);
    // --- Front face ---
    float tnc=max(tn,0.001);
    vec3 Pn=eye+rd*tnc;
    vec3 Nn=cubeNormal(Pn);
    vec3 V=normalize(eye-Pn);
    // Fresnel: transparent at center, reflective at grazing angles
    float f0=0.04; // glass IOR base
    float fresnel=f0+(1.0-f0)*pow(1.0-abs(dot(Nn,V)),5.0);
    // --- Back face (visible through glass) ---
    vec3 Pf=eye+rd*tf;
    vec3 Nf=-cubeNormal(Pf); // inward normal for back face
    float ndlBack=abs(dot(Nf,L))*0.5+0.5;
    vec3 backCol=vec3(0.2,0.55,0.95)*(0.5+0.5*ndlBack);
    // --- Front face specular (broader highlight) ---
    vec3 H=normalize(L+V);
    float spec=pow(max(dot(Nn,H),0.0),40.0);
    vec3 specCol=vec3(0.8,0.9,1.0)*spec*0.7;
    // --- Front face reflection (constant at grazing, sky-like) ---
    vec3 reflCol=vec3(0.25,0.45,0.8)*0.4;
    vec3 frontCol=reflCol+specCol;
    // --- Combine: center=transparent(back face), edge=reflective(front) ---
    vec3 col=mix(backCol,frontCol,fresnel);
    outColor=vec4(col*uLightColor,alpha);
}
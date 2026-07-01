#version 460
// Vol.04-14 Glass: 双面渲染玻璃效果
// Reference: Shininess=0.7 fixed-function → specPower=128 (2^(Shininess*10))
//            SeparateSpecular On, Combine Primary * Texture
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};
bool hitSphere(vec3 ro,vec3 rd,float r,out float t){
    float b=dot(ro,rd),c=dot(ro,ro)-r*r,h=b*b-c;
    if(h<0.0)return false;h=sqrt(h);t=-b-h;return t>0.001;
}
void main(){
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float ar=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=ar;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float b=dot(eye,rd),c=dot(eye,eye)-1.0,h=b*b-c;
    if(h<0.0){outColor=vec4(0.05,0.05,0.08,1.0);return;}
    float t1=-b-sqrt(h);float t2=-b+sqrt(h);
    if(t1<0.001)t1=t2;if(t1<0.001){outColor=vec4(0.05,0.05,0.08,1.0);return;}
    // Back face (Cull Front)
    vec3 Pb=eye+rd*t2;vec3 Nb=-normalize(Pb);
    vec3 L=normalize(uLightDir);vec3 Vb=normalize(eye-Pb);
    vec2 suvB=vec2(atan(Pb.z,Pb.x)*0.1591549+0.5,acos(clamp(Pb.y,-1.0,1.0))*0.3183099);
    vec3 texB=texture(uInputTex,suvB).rgb;
    float ndlB=max(dot(Nb,L),0.0);
    vec3 ambB=vec3(0.15);vec3 diffB=uLightColor*ndlB;
    vec3 Rb=reflect(-L,Nb);
    // Shininess=0.7 → specPower=128 (2^(0.7*10)=2^7=128)
    float specB=pow(max(dot(Rb,Vb),0.0),128.0);
    vec3 backColor=texB*(ambB+diffB+uLightColor*specB);
    // Front face (Cull Back)
    vec3 Pf=eye+rd*t1;vec3 Nf=normalize(Pf);vec3 Vf=normalize(eye-Pf);
    vec2 suvF=vec2(atan(Pf.z,Pf.x)*0.1591549+0.5,acos(clamp(Pf.y,-1.0,1.0))*0.3183099);
    vec3 texF=texture(uInputTex,suvF).rgb;
    float ndlF=max(dot(Nf,L),0.0);
    vec3 ambF=vec3(0.15);vec3 diffF=uLightColor*ndlF;
    vec3 Rf=reflect(-L,Nf);
    float specF=pow(max(dot(Rf,Vf),0.0),128.0);
    vec3 frontColor=texF*(ambF+diffF+uLightColor*specF);
    // Blend SrcAlpha OneMinusSrcAlpha
    float alpha=clamp(P0,0.05,1.0);
    vec3 col=mix(backColor,frontColor,alpha);
    outColor=vec4(col,alpha);
}
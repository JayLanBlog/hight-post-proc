#version 460
// Vol.04-17 Vegetation: two-pass cull-off with alpha test + transparent edges
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};
bool hit(vec3 ro,vec3 rd,float r,out float t){
    float b=dot(ro,rd),c=dot(ro,ro)-r*r,h=b*b-c;
    if(h<0.0)return false;h=sqrt(h);t=-b-h;return t>0.001;
}
float hash(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}
float noise(vec2 p){vec2 i=floor(p),f=fract(p);f=f*f*(3.0-2.0*f);return mix(mix(hash(i),hash(i+vec2(1,0)),f.x),mix(hash(i+vec2(0,1)),hash(i+vec2(1,1)),f.x),f.y);}
void main(){
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float a=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=a;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    // Hit both front and back faces (Cull Off)
    float t1,t2;
    float b=dot(eye,rd),c=dot(eye,eye)-1.0,h=b*b-c;
    if(h<0.0){outColor=vec4(0.02,0.02,0.04,1);return;}
    t1=-b-sqrt(h);t2=-b+sqrt(h);
    if(t1<0.001)t1=t2;if(t1<0.001){outColor=vec4(0.02,0.02,0.04,1);return;}
    vec3 P=eye+rd*t1;vec3 N=normalize(P);vec3 V=normalize(eye-P);
    vec3 L=normalize(uLightDir);
    float ndl=abs(dot(N,L))*0.5+0.5;
    // Vegetation pattern: hash-based leaf mask
    float leaf=hash(floor(P.xz*5.0+0.2))*hash(floor(P.yz*5.0+0.7));
    float threshold=P0;
    // Green with variation
    vec3 col=mix(vec3(0.1,0.3,0.05),vec3(0.2,0.7,0.1),hash(floor(P.xy*4.0)));
    if(leaf<threshold)outColor=vec4(0);
    else outColor=vec4(col*ndl*uLightColor,1);
}
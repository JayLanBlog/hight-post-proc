#version 460
// Vol.05-23 Glass v3: Dual pass - Pass1 AlphaBlend(texture*primary*2) + Pass2 Additive(cubemap)
// Reference: Pass1 Blend SrcAlpha OneMinusSrcAlpha, combine texture*primary double, texture*primary
//            Pass2 Blend One One, combine texture (cubemap)
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=2) uniform sampler2D uAuxTex;
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
vec3 tex(vec2 uv,float scale){float n=noise(uv*scale);return vec3(0.3+0.7*n);}
void main(){
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float a=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=a;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float t;if(!hit(eye,rd,1.0,t)){outColor=vec4(0.02,0.02,0.04,1);return;}
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 V=normalize(eye-P);vec3 L=normalize(uLightDir);
    vec2 suv=vec2(atan(P.z,P.x)*0.1591549+0.5,acos(clamp(P.y,-1.0,1.0))*0.3183099);
    // Pass 1: texture * primary * 2 (alpha blend)
    float ndl = max(dot(N, L), 0.0);
    vec3 ambient = vec3(0.15);
    vec3 primary = uLightColor * ndl + ambient;
    vec3 texColor = tex(suv, 8.0);
    vec3 pass1 = texColor * primary * 2.0;
    // Pass 2: cubemap reflection (additive, Blend One One)
    vec3 R = reflect(-V, N);
    float skyGrad = clamp(R.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 skyTop = vec3(0.3, 0.6, 1.0);
    vec3 skyHorizon = vec3(0.8, 0.9, 1.0);
    vec3 ground = vec3(0.15, 0.22, 0.35);
    vec3 refl = mix(ground, mix(skyHorizon, skyTop, skyGrad), skyGrad);
    // Combined: pass1 (alpha blended) + pass2 (additive cubemap)
    vec3 col = pass1 + refl;
    float alpha = P3;
    outColor = vec4(col, alpha);
}
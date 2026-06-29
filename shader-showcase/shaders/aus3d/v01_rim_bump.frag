#version 460
// Vol.01 Rim + Bump — diffuse with procedural bump and rim glow
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
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
    vec3 P=eye+rd*t;vec3 N=normalize(P);
    // Bump perturbation from checkerboard texture (4 repeats)
    vec2 suv=vec2(atan(P.z,P.x)*0.1591549+0.5,acos(clamp(P.y,-1.0,1.0))*0.3183099);
    float bump=texture(uInputTex,suv*4.0).r;
    vec3 N2=normalize(N+vec3((bump-0.5)*0.3,(bump-0.5)*0.3,0.0));
    // Diffuse
    vec3 L=normalize(uLightDir);
    float NdotL=max(dot(N2,L),0.0);
    vec3 tex=texture(uInputTex,suv).rgb;
    vec3 diffuse=tex*NdotL*uLightColor;
    // Rim glow
    vec3 V=normalize(eye-P);
    float rim=1.0-abs(dot(N,V));
    float glow=pow(max(rim,0.001),P0);
    vec3 rimColor=vec3(P1,P2,P3);
    vec3 col=diffuse+rimColor*glow*0.8;
    outColor=vec4(col,1);
}

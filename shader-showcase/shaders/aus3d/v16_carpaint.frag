#version 460
// CarPaint — smooth metallic auto paint
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(binding=2) uniform sampler2D uAuxTex; // MatCap纹理
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
    float ar=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=ar;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float t;if(!hit(eye,rd,1.0,t)){outColor=vec4(0.02,0.02,0.04,1);return;}
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 V=normalize(eye-P);vec3 L=normalize(uLightDir);
    vec3 H=normalize(L+V);

    float ndl=dot(N,L)*0.5+0.5;
    float nh=dot(N,H);

    // 1. Smooth metallic gradient base — no hard cut
    vec3 dark=vec3(0.06,0.10,0.24);
    vec3 mid=vec3(0.10,0.20,0.48);
    vec3 lit=vec3(0.18,0.30,0.60);
    vec3 base=mix(dark,mid,smoothstep(0.3,0.7,ndl));
    base=mix(base,lit,smoothstep(0.6,0.95,ndl));

    // 2. Soft specular — wide gaussian-like falloff
    float specSoft=pow(max(nh,0.0),P0*30.0+8.0);
    vec3 specCol=vec3(1.0,0.96,0.88)*specSoft*0.5;

    // 3. Broad clearcoat sheen
    float coat=pow(max(nh,0.0),P0*4.0+2.0)*0.12;
    vec3 coatCol=vec3(0.25,0.48,0.78)*coat;

    // 4. Fresnel rim — smooth deep blue
    float fres=pow(1.0-max(dot(N,V),0.0),3.5);
    vec3 rimCol=vec3(0.18,0.38,0.72)*fres*0.4;

    // 5. Sparse fine flakes — larger cells, softer threshold
    vec2 fc=vec2(dot(N,rt),dot(N,up))*0.5+0.5;
    float f1=texture(uInputTex,fc*10.0+uTime*0.012).r;
    float f2=texture(uInputTex,fc*17.0-uTime*0.008).r;
    float flakeRaw=(f1+f2)*0.5;
    float flake=smoothstep(0.55,0.75,flakeRaw)*0.08;

    // 6. Fake top-sky reflection
    float env=dot(N,vec3(0,1,0))*0.5+0.5;
    base=mix(base,base*vec3(0.7,0.8,1.1),env*0.12);

    // 7. Gentle light wrap
    float wrap=dot(N,L)*0.35+0.65;

    vec3 col=(base+rimCol+coatCol)*wrap+specCol+flake;
    // MatCap纹理采样增强
    float NdotV=abs(dot(N,V));
    vec2 matcapUV=vec2(NdotV*0.5+0.5,ndl*0.5+0.5);
    vec3 matcapColor=texture(uAuxTex,matcapUV).rgb;
    col=mix(col,matcapColor,0.3); // 30% MatCap混合
    outColor=vec4(col*uLightColor,1);
}

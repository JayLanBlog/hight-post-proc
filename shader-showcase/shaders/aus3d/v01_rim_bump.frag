#version 460
// Vol.01 Bump+Rim — rough stone with cracks
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=2) uniform sampler2D uAuxTex; // 噪声纹理
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};
bool hit(vec3 ro,vec3 rd,float r,out float t){
    float b=dot(ro,rd),c=dot(ro,ro)-r*r,h=b*b-c;
    if(h<0.0)return false;h=sqrt(h);t=-b-h;return t>0.001;
}
float hash(float n){return fract(sin(n)*43758.5453);}
float noise2(vec2 p){
    vec2 i=floor(p);vec2 f=fract(p);
    f=f*f*(3.0-2.0*f);
    return mix(mix(hash(i.x+hash(i.y)*157.0),hash(i.x+1.0+hash(i.y)*157.0),f.x),
               mix(hash(i.x+hash(i.y+1.0)*157.0),hash(i.x+1.0+hash(i.y+1.0)*157.0),f.x),f.y);
}
float fbm(vec2 p){
    float v=0.0,a=0.5;float f=1.0;
    for(int i=0;i<5;i++){v+=noise2(p*f)*a;f*=2.0;a*=0.55;}
    return v;
}
void main(){
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float a=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=a;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float t;if(!hit(eye,rd,1.0,t)){outColor=vec4(0.04,0.03,0.06,1);return;}
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 L=normalize(uLightDir);vec3 V=normalize(eye-P);
    // Bump perturbation from noise texture
    vec2 bumpUV=vec2(atan(N.z,N.x),asin(N.y))*vec2(0.5/3.14159,1.0/3.14159)+0.5;
    vec3 bump=texture(uAuxTex,bumpUV).rgb*2.0-1.0;
    N=normalize(N+bump*P0*0.15);
    vec2 suv=vec2(atan(P.z,P.x)*0.1591549+0.5,acos(clamp(P.y,-1.0,1.0))*0.3183099);
    vec2 bp=suv*8.0;
    
    // Multi-octave height
    float h=fbm(bp);
    
    // Block grid: subtle 4x4 stone block pattern
    vec2 bi=floor(suv*4.0);vec2 bf=fract(suv*4.0);
    float bw=0.04;
    float crack=smoothstep(0.0,bw,bf.x)+smoothstep(0.0,bw,bf.y)
             +smoothstep(0.0,bw,1.0-bf.x)+smoothstep(0.0,bw,1.0-bf.y);
    crack=min(crack,1.0)*0.25;
    // Random offset per block so they don't look uniform
    float rnd=hash(bi.x+hash(bi.y)*73.0);
    float crackRand=crack*(0.5+rnd*0.5);
    
    // Combine organic noise + structured cracks
    float finalH=h*0.8+crackRand*0.35;
    
    // Derivative via central difference
    float eps=0.003;
    float hx=(fbm(bp+vec2(eps*8.0,0))*0.8-fbm(bp-vec2(eps*8.0,0))*0.8)/eps;
    float hy=(fbm(bp+vec2(0,eps*8.0))*0.8-fbm(bp-vec2(0,eps*8.0))*0.8)/eps;
    
    vec3 T=normalize(cross(N,vec3(0,1,0)));vec3 B=cross(N,T);
    vec3 Nb=normalize(N-T*hx*0.22-B*hy*0.22);
    
    // Stone color: darker at cracks, lighter on blocks
    vec3 stone=vec3(0.48,0.42,0.35);
    vec3 crackCol=vec3(0.15,0.13,0.10);
    vec3 base=mix(stone,crackCol,crackRand);
    
    // Ambient 35% + diffuse 65% — deeper shadows but no black
    float ndl=dot(Nb,L)*0.5+0.5;
    float lit=ndl*0.65+0.35;
    vec3 col=base*lit*uLightColor;
    
    // Very subtle rim — stone doesn't glow
    float rim=1.0-abs(dot(Nb,V));
    col+=vec3(P1,P2,P3)*pow(max(rim,0.001),P0)*0.12;
    
    outColor=vec4(col,1);
}

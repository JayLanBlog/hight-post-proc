#version 460
// Vol.01 Bump+Rim — soft organic bump
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};
bool hit(vec3 ro,vec3 rd,float r,out float t){
    float b=dot(ro,rd),c=dot(ro,ro)-r*r,h=b*b-c;
    if(h<0.0)return false;h=sqrt(h);t=-b-h;return t>0.001;
}

float hash(float n){return fract(sin(n)*43758.5453);}
float noise(vec2 p){
    vec2 i=floor(p);vec2 f=fract(p);
    f=f*f*(3.0-2.0*f);
    float a=hash(i.x+hash(i.y)*157.0);
    float b=hash(i.x+1.0+hash(i.y)*157.0);
    float c=hash(i.x+hash(i.y+1.0)*157.0);
    float d=hash(i.x+1.0+hash(i.y+1.0)*157.0);
    return mix(mix(a,b,f.x),mix(c,d,f.x),f.y);
}

void main(){
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float a=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=a;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float t;if(!hit(eye,rd,1.0,t)){outColor=vec4(0.04,0.03,0.06,1);return;}
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 L=normalize(uLightDir);vec3 V=normalize(eye-P);
    vec2 suv=vec2(atan(P.z,P.x)*0.1591549+0.5,acos(clamp(P.y,-1.0,1.0))*0.3183099);
    
    // Smooth multi-octave noise for height
    vec2 p=suv*14.0;
    float h=0.0;
    h+=noise(p)*0.6;
    h+=noise(p*2.1+0.7)*0.3;
    h+=noise(p*4.3+1.4)*0.1;
    
    // Derivative via finite difference
    float eps=0.003;
    float hx=(noise(p+vec2(eps*14.0,0))*0.6+noise((p+vec2(eps*14.0,0))*2.1+0.7)*0.3+noise((p+vec2(eps*14.0,0))*4.3+1.4)*0.1
            -(noise(p-vec2(eps*14.0,0))*0.6+noise((p-vec2(eps*14.0,0))*2.1+0.7)*0.3+noise((p-vec2(eps*14.0,0))*4.3+1.4)*0.1))/eps;
    float hy=(noise(p+vec2(0,eps*14.0))*0.6+noise((p+vec2(0,eps*14.0))*2.1+0.7)*0.3+noise((p+vec2(0,eps*14.0))*4.3+1.4)*0.1
            -(noise(p-vec2(0,eps*14.0))*0.6+noise((p-vec2(0,eps*14.0))*2.1+0.7)*0.3+noise((p-vec2(0,eps*14.0))*4.3+1.4)*0.1))/eps;
    
    vec3 T=normalize(cross(N,vec3(0,1,0)));vec3 B=cross(N,T);
    vec3 Nb=normalize(N-T*hx*0.15-B*hy*0.15);
    
    // Warm stone colour with gentle height variation
    vec3 base=mix(vec3(0.38,0.34,0.28),vec3(0.52,0.46,0.38),h*0.6);
    // Ambient-rich lighting — never goes completely dark
    float ndl=dot(Nb,L)*0.5+0.5;
    float lit=ndl*0.5+0.5;  // 50% ambient minimum
    vec3 col=base*lit*uLightColor;
    
    // Soft rim for shape
    float rim=1.0-abs(dot(Nb,V));
    col+=vec3(P1,P2,P3)*pow(max(rim,0.001),P0)*0.2;
    
    outColor=vec4(col,1);
}

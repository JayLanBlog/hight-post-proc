#version 460
// Vol.10 Oil Paint: block-based normal perturbation + color variation per block
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};
bool hit(vec3 ro,vec3 rd,float r,out float t){
    float b=dot(ro,rd),c=dot(ro,ro)-r*r,h=b*b-c;
    if(h<0.0)return false;h=sqrt(h);t=-b-h;return t>0.001;
}
float hash3(vec3 p){return fract(sin(dot(p,vec3(127.1,311.7,74.7)))*43758.5453);}

void main(){
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float ar=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=ar;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float t;if(!hit(eye,rd,1.0,t)){outColor=vec4(0.02,0.02,0.04,1);return;}
    vec3 P=eye+rd*t;vec3 N=normalize(P);
    vec3 L=normalize(uLightDir);

    // Block size: P0=0.5->0.05, P0=1.0->0.1 (in 3D space, ~3-6 degrees on sphere)
    float blockSize=0.03+P0*0.07;
    // Perturbation strength: P1=1.0->0.8 radians (~46 degrees)
    float perturbStrength=P1*0.8;

    // Build tangent space
    vec3 T=normalize(cross(N,vec3(0,1,0)));
    if(length(cross(N,vec3(0,1,0)))<0.001)T=normalize(cross(N,vec3(1,0,0)));
    vec3 B=normalize(cross(N,T));

    // Block ID
    vec3 blockId=floor(P/blockSize);
    float rn=hash3(blockId);

    // Perturb normal per block
    float angleU=(rn-0.5)*perturbStrength*2.0;
    float angleV=(hash3(blockId+vec3(0.37,0.19,0.53))-0.5)*perturbStrength*2.0;
    vec3 Np=normalize(N+T*angleU+B*angleV);

    // Lighting with perturbed normal
    float ndl=dot(Np,L)*0.5+0.5;

    // Rich color per block: green/yellow/brown palette
    float tex=hash3(blockId+vec3(0.73,0.91,0.47));
    float hue=hash3(blockId+vec3(0.11,0.23,0.59));
    vec3 col=mix(vec3(0.15,0.5,0.08),vec3(0.6,0.7,0.2),tex)*ndl;

    outColor=vec4(col*uLightColor,1);
}
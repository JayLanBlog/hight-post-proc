#version 460
// Vol.09 Water Drop: 水幕/水珠特效 - 涟漪纹路叠加在球体表面
// P0: speed, P1: distortion strength
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
    float ar=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=ar;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float speed=P0*0.3;

    // === 渲染球体（无扭曲） ===
    float t;bool hitSphere=hit(eye,rd,1.0,t);
    vec3 sphereCol;
    if(hitSphere){
        vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 L=normalize(uLightDir);
        float ndl=dot(N,L)*0.5+0.5;
        sphereCol=vec3(0.9,0.4,0.2)*ndl;
    }else{
        sphereCol=vec3(0.0);
    }

    // === 涟漪纹路计算 ===
    float ripple=0.0;
    float dropHighlight=0.0;
    for(int i=0;i<6;i++){
        float fi=float(i);
        vec2 dropCenter=vec2(
            sin(fi*1.3+uTime*0.15+fi*2.0)*0.7,
            cos(fi*1.7+uTime*0.2+fi*1.5)*0.6
        );
        float dist=length(uv-dropCenter);
        float ringFreq=5.0+fi*0.5;
        float wave=sin(dist*ringFreq-uTime*1.5+fi*0.5)*exp(-dist*0.55);
        ripple+=wave*0.28;
        float drop=exp(-dist*dist*4.5)*0.4;
        dropHighlight+=drop;
    }

    // === 涟漪叠加 ===
    // 涟漪亮纹：白色/青色
    float rippleBright=abs(ripple);
    vec3 rippleColor=vec3(0.35,0.55,0.85)*rippleBright;
    // 水滴高光
    vec3 dropColor=vec3(0.30,0.45,0.75)*dropHighlight;
    // 暗纹（波谷）
    float rippleDark=max(-ripple,0.0)*0.5;

    vec3 col;
    if(hitSphere){
        // 球体上的涟漪：叠加亮纹 + 暗纹
        col=sphereCol;
        col+=rippleColor*0.6;
        col-=sphereCol*rippleDark;
        col+=dropColor*0.3;
    }else{
        // 背景上的涟漪
        vec3 bg=vec3(0.02,0.03,0.08);
        bg+=rippleColor*0.2;
        bg+=dropColor*0.4;
        col=bg;
    }

    outColor=vec4(col*uLightColor,1);
}
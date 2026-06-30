#version 460
// Pass1: 用偏移UV渲染球体 + 背景涟漪
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex; // UV偏移纹理(RG=dx,dy)
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

bool hit(vec3 ro,vec3 rd,float r,out float t){
    float b=dot(ro,rd),c=dot(ro,ro)-r*r,h=b*b-c;
    if(h<0.0)return false;h=sqrt(h);t=-b-h;return t>0.001;
}

void main() {
    // 读取UV偏移
    vec2 offset = texture(uInputTex, vUV).rg * 2.0;
    
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float ar=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=ar;
    uv += offset; // 应用偏移
    
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float t;
    if(!hit(eye,rd,1.0,t)){
        // 背景涟漪
        float ripple = sin(vUV.y * 20.0 + uTime) * 0.1 + sin(vUV.x * 15.0 - uTime * 0.7) * 0.1;
        vec3 bg = vec3(0.02,0.04,0.10) + vec3(0.06,0.10,0.25) * abs(ripple);
        outColor = vec4(bg,1.0);
        return;
    }
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 L=normalize(uLightDir);
    float ndl=dot(N,L)*0.5+0.5;
    vec3 col=vec3(0.9,0.35,0.15)*ndl;
    outColor=vec4(col*uLightColor,1.0);
}
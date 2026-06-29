#version 460
// Vol.16 CarPaint — procedural metallic matcap
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
    // Matcap UV from normal in view space
    vec3 mrt=normalize(cross(vec3(0,1,0),normalize(-eye)));
    vec3 mup=normalize(cross(normalize(-eye),mrt));
    vec2 mc=vec2(dot(N,mrt)*0.5+0.5,dot(N,mup)*0.5+0.5);
    // Procedural metallic car paint: R=reflection gradient, G=flakes, B=gloss band
    float spec=pow(max(dot(N,normalize(-eye)),0.0),P0*50.0+8.0);
    // Horizontal gradient (reflection horizon)
    float horiz=smoothstep(0.3,0.7,mc.y)*smoothstep(0.3,0.7,1.0-mc.y);
    // Color: deep blue metallic with specular highlight
    vec3 base=vec3(0.05,0.1,0.3);
    vec3 reflectCol=vec3(0.2,0.35,0.7);
    vec3 col=mix(base,reflectCol,horiz*0.7);
    // Bright specular spot near center
    col+=vec3(1.0,0.9,0.7)*spec*0.6;
    // Fake metallic flakes
    float flake=texture(uInputTex,mc*3.0+uTime*0.01).r;
    col+=flake*0.05;
    outColor=vec4(col,1);
}

#version 460
// Vol.03-8 Alpha+Emission: 纹理Alpha通道作自发光遮罩
// Reference: Material Diffuse(1,1,1,1) Ambient(1,1,1,1), Lighting On
//            constantColor(1,1,1,1), combine constant lerp(texture) previous
//            combine previous * texture
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
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float t;if(!hit(eye,rd,1.0,t)){outColor=vec4(0.02,0.02,0.04,1);return;}
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 L=normalize(uLightDir);
    vec2 suv=vec2(atan(P.z,P.x)*0.1591549+0.5,acos(clamp(P.y,-1.0,1.0))*0.3183099);
    vec4 tex=texture(uInputTex,suv); // RGB=纹理颜色, A=自发光遮罩
    // Reference: Material Diffuse(1,1,1,1) Ambient(1,1,1,1), Lighting On
    // constantColor(1,1,1,1), combine constant lerp(texture) previous
    float ndl=max(dot(N,L),0.0);
    vec3 ambient=vec3(1.0); // Reference: Ambient(1,1,1,1)
    vec3 diffuse=uLightColor*ndl;
    vec3 vertexLight=ambient+diffuse;
    // lerp between vertexLight and white (1,1,1) using texAlpha
    vec3 result=mix(vertexLight,vec3(1.0),tex.a);
    // multiply by texture color
    result=result*tex.rgb;
    outColor=vec4(result,1.0);
}
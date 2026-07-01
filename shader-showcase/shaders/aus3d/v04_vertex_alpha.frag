#version 460
// Vol.04-16 VertexLight+Alpha: 顶点光照 + 可调Alpha测试
// Reference: AlphaTest Greater[_Cutoff], Material Diffuse[_Color] Ambient[_Color] Shininess[_Shininess]
//            Specular[_SpecColor] Emission[_Emission], Lighting On, combine texture * primary
// Shininess=0.7 → specPower=128
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
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 L=normalize(uLightDir);vec3 V=normalize(eye-P);
    vec2 suv=vec2(atan(P.z,P.x)*0.1591549+0.5,acos(clamp(P.y,-1.0,1.0))*0.3183099);
    vec4 tex=texture(uInputTex,suv); // RGB=纹理颜色, A=透明度
    // Reference: AlphaTest Greater[_Cutoff]
    float cutoff=P0;
    if(tex.a<cutoff)discard;
    // Reference: combine texture * primary
    // primary = ambient + diffuse + specular (Shininess=0.7 → specPower=128)
    float ndl=max(dot(N,L),0.0);
    vec3 ambient=vec3(0.15);
    vec3 diffuse=uLightColor*ndl;
    vec3 R=reflect(-L,N);
    float spec=pow(max(dot(R,V),0.0),128.0);
    vec3 specular=uLightColor*spec;
    vec3 primary=ambient+diffuse+specular;
    outColor=vec4(tex.rgb*primary,1.0);
}
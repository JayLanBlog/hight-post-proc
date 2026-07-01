#version 460
// Vol.06-31 DetailTexture: Lambert + baseTex * detailTex * 2
// Reference: Surface Shader Lambert, o.Albedo = mainTex * detailTex * 2
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=2) uniform sampler2D uAuxTex;
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
vec3 tex(vec2 uv,float scale){float n=noise(uv*scale);return vec3(0.2+0.8*n,0.3+0.7*noise(uv*scale+1.5),0.4+0.6*noise(uv*scale+3.0));}
void main(){
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float a=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=a;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float t;if(!hit(eye,rd,1.0,t)){outColor=vec4(0.02,0.02,0.04,1);return;}
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 L=normalize(uLightDir);
    vec2 suv=vec2(atan(P.z,P.x)*0.1591549+0.5,acos(clamp(P.y,-1.0,1.0))*0.3183099);
    // Reference: o.Albedo = tex2D(_MainTex, uv).rgb; o.Albedo *= tex2D(_Detail, uv).rgb * 2;
    vec3 mainTex = tex(suv, 4.0);
    vec3 detailTex = tex(suv, 16.0) * 2.0;
    vec3 albedo = mainTex * detailTex;
    // Lambert lighting (not Half-Lambert)
    float ndl = max(dot(N, L), 0.0);
    vec3 ambient = vec3(0.15);
    // Unity Lambert: c.rgb = s.Albedo * _LightColor0 * (NdotL * atten * 2)
    // → albedo * uLightColor * ndl * 2.0 + ambient
    vec3 col = albedo * (uLightColor * ndl * 2.0 + ambient);
    outColor = vec4(col, 1.0);
}
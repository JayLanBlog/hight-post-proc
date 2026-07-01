#version 460
// Vol.06-32 FullCombo: bump + rim + detail texture + color tint
// Reference: Surface Shader Lambert + finalcolor tint + bump + detail*2 + rim emission
//            o.Emission = _RimColor.rgb * pow(rim, _RimPower), rim = 1.0 - saturate(dot(viewDir, N))
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
vec3 bumpNormal(vec3 N,vec3 P){
    float eps=0.01;vec2 suv=vec2(atan(P.z,P.x)*0.1591549+0.5,acos(clamp(P.y,-1.0,1.0))*0.3183099);
    float h=noise(suv*8.0);float hx=noise((suv+vec2(eps,0))*8.0);float hy=noise((suv+vec2(0,eps))*8.0);
    vec3 T=normalize(cross(N,vec3(0,1,0)));if(length(T)<0.01)T=normalize(cross(N,vec3(1,0,0)));
    vec3 B=cross(N,T);
    float s=0.3;return normalize(N+s*((hx-h)/eps*T+(hy-h)/eps*B));
}
vec3 tex(vec2 uv,float s){float n=noise(uv*s);return vec3(0.2+0.8*n,0.3+0.7*noise(uv*s+1.5),0.4+0.6*noise(uv*s+3.0));}
void main(){
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float a=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=a;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float t;if(!hit(eye,rd,1.0,t)){outColor=vec4(0.02,0.02,0.04,1);return;}
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 V=normalize(eye-P);
    vec2 suv=vec2(atan(P.z,P.x)*0.1591549+0.5,acos(clamp(P.y,-1.0,1.0))*0.3183099);
    // Reference: o.Albedo = tex2D(_MainTex, uv).rgb; o.Albedo *= tex2D(_Detail, uv).rgb * 2;
    vec3 baseTex=tex(suv,4.0);
    vec3 detailTex=tex(suv,16.0)*2.0;
    vec3 albedo=baseTex*detailTex;
    // Bump normal perturbation
    vec3 Nbp=bumpNormal(N,P);
    vec3 L=normalize(uLightDir);
    // Lambert lighting
    float ndl=max(dot(Nbp,L),0.0);
    vec3 ambient=vec3(0.15);
    vec3 diffuse=albedo*uLightColor*ndl;
    vec3 col=diffuse+albedo*ambient;
    // Reference: finalcolor tint → color *= _ColorTint
    vec3 tint=vec3(P0,P1,P2);
    col*=tint;
    // Reference: o.Emission = _RimColor.rgb * pow(rim, _RimPower)
    // rim = 1.0 - saturate(dot(viewDir, N)) (NOT abs!)
    float rim=1.0-clamp(dot(Nbp,V),0.0,1.0);
    vec3 rimCol=vec3(P3,P4,P5)*pow(rim,3.0);
    col+=rimCol;
    outColor=vec4(col,1);
}
#version 460
// Vol.07 Specular: 自定义高光
// Reference: c.rgb = (s.Albedo * _LightColor0.rgb * diff + _LightColor0.rgb * spec) * (atten * 2)
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
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
    vec3 H=normalize(L+V);
    // Reference: Lambert diffuse
    float diff=max(0.0,dot(N,L));
    // Reference: Blinn-Phong specular, pow 48, spec color = _LightColor0.rgb
    float spec=pow(max(dot(N,H),0.0),48.0)*P0;
    vec3 albedo=vec3(0.8,0.75,0.65);
    // Reference: (albedo * lightColor * diff + lightColor * spec) * 2
    vec3 col=(albedo*uLightColor*diff+uLightColor*spec)*2.0;
    outColor=vec4(col,1);
}
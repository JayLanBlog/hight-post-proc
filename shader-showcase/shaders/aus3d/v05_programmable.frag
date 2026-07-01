#version 460
// Vol.05 Programmable: 可编程Phong光照
// Reference: specularReflection = atten * _LightColor0.rgb * _SpecColor.rgb * NdotL * pow(RdotV, _Shininess)
//            lightFinal = diffuseReflection + specularReflection + UNITY_LIGHTMODEL_AMBIENT
//            o.col = lightFinal * _Color.rgb
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
    float NdotL=max(0.0,dot(N,L));
    // Reference: diffuseReflection = _LightColor0.rgb * NdotL
    vec3 diffuseReflection=uLightColor*NdotL;
    // Reference: specularReflection = _LightColor0.rgb * _SpecColor.rgb * NdotL * pow(RdotV, _Shininess)
    // _Shininess default=10, _SpecColor default=(1,1,1,1)
    vec3 specularReflection=vec3(0.0);
    if(NdotL>0.0){
        vec3 R=reflect(-L,N);
        float spec=pow(max(dot(R,V),0.0),10.0);
        // Reference: full RGB specular, not just R channel
        specularReflection=uLightColor*spec*NdotL;
    }
    // Reference: lightFinal = diffuseReflection + specularReflection + UNITY_LIGHTMODEL_AMBIENT
    vec3 ambient=vec3(0.15);
    vec3 lightFinal=diffuseReflection+specularReflection+ambient;
    // Reference: o.col = lightFinal * _Color.rgb
    vec3 col=lightFinal*vec3(P0,P1,P2);
    outColor=vec4(col,1);
}
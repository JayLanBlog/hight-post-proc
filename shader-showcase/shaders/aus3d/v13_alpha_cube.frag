#version 460
// Vol.13 Alpha Cube: 透明立方体，无光照，alpha=P0
// Reference: Volume 13 SimpleAlphaShader / ColorChangeAlpha
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};
bool hitCube(vec3 ro,vec3 rd,out float tn,out float tf){
    vec3 t1=(vec3(-0.4)-ro)/rd; vec3 t2=(vec3(0.4)-ro)/rd;
    vec3 tnv=min(t1,t2); vec3 tfv=max(t1,t2);
    tn=max(max(tnv.x,tnv.y),tnv.z);
    tf=min(min(tfv.x,tfv.y),tfv.z);
    return tf>0.001&&tn<tf;
}
void main(){
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float ar=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=ar;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float tn,tf;
    if(!hitCube(eye,rd,tn,tf)){outColor=vec4(0.02,0.02,0.04,1);return;}
    // Simple transparent cube: no lighting, alpha = P0
    // Reference: Queue=Transparent, Blend SrcAlpha OneMinusSrcAlpha, ZWrite Off
    outColor=vec4(0.3, 1.0, 0.1, P0);
}
#version 460
// Vol.16 CarPaint: MatCap车漆 — mainColor * matCapColor * 2.0
// Reference: finalColor = mainColor * matCapColor * 2.0
// MatCap UV: 法线从模型空间转换到观察空间
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=2) uniform sampler2D uAuxTex; // MatCap纹理
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
    float ar=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=ar;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float t;if(!hit(eye,rd,1.0,t)){outColor=vec4(0.02,0.02,0.04,1);return;}
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 V=normalize(eye-P);
    // MatCap UV: 法线转换到观察空间 [0,1]
    // Reference: matCapCoords.z = dot(MV_IT[0], normal); matCapCoords.w = dot(MV_IT[1], normal);
    // Simplified: use normal in view space
    float mx = dot(N, rt);
    float my = dot(N, up);
    vec2 matcapUV = vec2(mx, my) * 0.5 + 0.5;
    vec3 matcapColor = texture(uAuxTex, matcapUV).rgb;
    // Reference: mainColor * matCapColor * 2.0
    vec3 mainColor = vec3(1.0, 0.3, 0.1); // car paint base red
    vec3 col = mainColor * matcapColor * 2.0;
    // P0: reflection strength
    col = mix(mainColor, col, P0);
    outColor = vec4(col, 1.0);
}
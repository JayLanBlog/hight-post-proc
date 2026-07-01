#version 460
// Vol.05-19 Blend Multiply: multiply blend mode — dst * src
// Reference: Blend DstColor Zero, SetTexture[_MainTex]{combine texture}
//            → output = framebuffer * texture (multiply blend)
// Approximation: use scene background as framebuffer, multiply by procedural pattern
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};
float hash(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}
bool hit(vec3 ro,vec3 rd,float r,out float t){
    float b=dot(ro,rd),c=dot(ro,ro)-r*r,h=b*b-c;
    if(h<0.0)return false;h=sqrt(h);t=-b-h;return t>0.001;
}
void main(){
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float a=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=a;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float t;if(!hit(eye,rd,1.0,t)){outColor=vec4(0.02,0.02,0.04,1);return;}
    vec3 P=eye+rd*t;
    vec2 suv=vec2(atan(P.z,P.x)*0.1591549+0.5,acos(clamp(P.y,-1.0,1.0))*0.3183099);
    // Reference: Blend DstColor Zero → dst * src + 0 * 0 = dst * src
    // dst = framebuffer (approximated by scene background), src = texture
    vec3 framebuffer = texture(uInputTex, suv).rgb;
    // Procedural multiply pattern on sphere surface
    float n = hash(suv * 100.0);
    vec3 pattern = mix(vec3(0.3, 0.2, 0.6), vec3(0.9, 0.8, 1.0), n);
    // Multiply blend: framebuffer * pattern
    outColor = vec4(framebuffer * pattern, 1.0);
}
#version 460
// Bleach Bypass
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
vec4 c=texture(uInputTex,vUV);float l=dot(c.rgb,vec3(0.222,0.707,0.071));vec3 g=vec3(l);float L=clamp(10.0*(l-0.45),0,1);vec3 r1=2.0*c.rgb*g;vec3 r2=1.0-2.0*(1.0-g)*(1.0-c.rgb);outColor=vec4(mix(c.rgb,mix(r1,r2,L),uParamFloat0),c.a);
}
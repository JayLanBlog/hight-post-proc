#version 460
// Roberts Neon — amplified for thumbnails
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
vec2 ts=vec2(uParamFloat0,uParamFloat1)/uResolution;
vec4 tl=texture(uInputTex,vUV+vec2(-ts.x,ts.y)),tr=texture(uInputTex,vUV+vec2(ts.x,ts.y));
vec4 bl=texture(uInputTex,vUV+vec2(-ts.x,-ts.y)),br=texture(uInputTex,vUV+vec2(ts.x,-ts.y));
float g=abs(dot(tl.rgb,vec3(0.3,0.59,0.11))-dot(br.rgb,vec3(0.3,0.59,0.11)));
g+=abs(dot(tr.rgb,vec3(0.3,0.59,0.11))-dot(bl.rgb,vec3(0.3,0.59,0.11)));
vec4 sc=texture(uInputTex,vUV);
float pulse=uParamFloat5*(1.0+0.3*sin(uTime*0.9));
float edge=clamp(g*pulse,0.0,1.0);
vec3 neon=vec3(uParamFloat2,uParamFloat3,uParamFloat4);
outColor=mix(sc,vec4(neon*edge+sc.rgb*(1.0-edge),1.0),edge);
}

#version 460
// Roberts Edge — amplified for thumbnails
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
vec2 ts=vec2(uParamFloat0,uParamFloat1)/uResolution;
vec2 tl=texture(uInputTex,vUV+vec2(-ts.x,ts.y)).rg,tr=texture(uInputTex,vUV+vec2(ts.x,ts.y)).rg;
vec2 bl=texture(uInputTex,vUV+vec2(-ts.x,-ts.y)).rg,br=texture(uInputTex,vUV+vec2(ts.x,-ts.y)).rg;
float grey_tl=dot(tl,vec2(0.299,0.587))+0.114*texture(uInputTex,vUV+vec2(-ts.x,ts.y)).b;
float grey_tr=dot(tr,vec2(0.299,0.587))+0.114*texture(uInputTex,vUV+vec2(ts.x,ts.y)).b;
float grey_bl=dot(bl,vec2(0.299,0.587))+0.114*texture(uInputTex,vUV+vec2(-ts.x,-ts.y)).b;
float grey_br=dot(br,vec2(0.299,0.587))+0.114*texture(uInputTex,vUV+vec2(ts.x,-ts.y)).b;
float g=abs(grey_tl-grey_br)+abs(grey_tr-grey_bl);
vec4 sc=texture(uInputTex,vUV);
float pulse=uParamFloat2*(1.0+0.3*sin(uTime*1.2));
float edge=clamp(g*pulse,0.0,1.0);
outColor=mix(sc,vec4(edge+sc.rgb*0.3,1.0),edge);
}

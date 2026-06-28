#version 460
// LED Pixelate — circular LED dots with visible gap
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
float ps=1.0/max(uParamFloat0,1.0);vec2 q=floor(vUV/ps)*ps+ps*0.5;vec2 d=(vUV-q)/ps;
// LED dot radius: smaller => more gap between dots
float threshold=0.25*clamp(uParamFloat1,0.1,2.0);
if(length(d)>threshold){
    // Add slight dark glow around dots instead of pure black
    float glow=smoothstep(threshold,min(threshold+0.15,0.6),length(d));
    vec4 col=texture(uInputTex,q);
    outColor=mix(col,vec4(0,0,0,1),glow);
}else{
    outColor=texture(uInputTex,q);
}
}

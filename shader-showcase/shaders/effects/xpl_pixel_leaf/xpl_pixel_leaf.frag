#version 460
// Leaf Pixelate — 3-petal leaf shape with visible boundary
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
float ps=1.0/max(uParamFloat0,1.0);vec2 q=floor(vUV/ps)*ps+ps*0.5;vec2 d=(vUV-q)/ps;
float a=atan(d.y,d.x);float r=length(d);
// 3-petal leaf: higher value at tips, lower at base
float leaf=abs(cos(a*3.0))*0.5+0.5;
// Scale: when r > the leaf boundary, draw black silhouette  
float boundary=leaf*clamp(uParamFloat3,0.1,4.0)*0.35;
if(r>boundary){
    vec4 col=texture(uInputTex,q);
    float edge=smoothstep(boundary,min(boundary+0.1,0.7),r);
    outColor=mix(col,vec4(0,0,0,1),edge);
}else{
    outColor=texture(uInputTex,q);
}
}

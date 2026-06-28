#version 460
// Circle Pixelate
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
float ps=1.0/max(uParamFloat0,1.0);float ratio=uResolution.y/uResolution.x;vec2 uv=vUV;uv.x/=ratio;vec2 coord=vec2(floor(uv.x/(ps*uParamFloat1))*uParamFloat1,floor(uv.y/(ps*uParamFloat2))*uParamFloat2);vec2 cc=coord*ps+ps*0.5;float dist=length(uv-cc)*uParamFloat0;cc.x*=ratio;vec4 sc=texture(uInputTex,cc);if(dist>uParamFloat3)sc=vec4(vec3(0),1.0);outColor=sc;
}
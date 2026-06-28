#version 460
// Diamond Pixelate
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
float ps=1.0/max(uParamFloat0,1.0);float dx=fract(vUV.x/ps)-0.5;float dy=fract(vUV.y/ps)-0.5;if(abs(dx)+abs(dy)>0.5*clamp(uParamFloat3,0,4)){outColor=vec4(vec3(0),1.0);}else{vec2 q=floor(vUV/ps)*ps+ps*0.5;outColor=texture(uInputTex,q);}
}
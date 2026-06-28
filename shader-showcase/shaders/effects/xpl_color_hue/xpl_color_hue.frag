#version 460
// Hue Shift
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
vec4 c=texture(uInputTex,vUV);float a=uParamFloat0*3.14159/180.0;float cs=cos(a),sn=sin(a);mat3 m=mat3(0.299+0.701*cs+0.168*sn,0.587-0.587*cs+0.330*sn,0.114-0.114*cs-0.497*sn,0.299-0.299*cs-0.328*sn,0.587+0.413*cs+0.035*sn,0.114-0.114*cs+0.292*sn,0.299-0.3*cs+1.25*sn,0.587-0.588*cs-1.05*sn,0.114+0.886*cs-0.203*sn);outColor=vec4(c.rgb*m,c.a);
}
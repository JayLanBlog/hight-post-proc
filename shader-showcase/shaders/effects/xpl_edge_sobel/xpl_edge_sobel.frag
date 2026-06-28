#version 460
// Sobel Edge — amplified for thumbnails
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
vec2 step=vec2(uParamFloat0,uParamFloat1)/uResolution;
vec2 ts=step;
float tl=dot(texture(uInputTex,vUV+vec2(-ts.x,ts.y)).rgb,vec3(0.3,0.59,0.11));
float ml=dot(texture(uInputTex,vUV+vec2(-ts.x,0.0)).rgb,vec3(0.3,0.59,0.11));
float bl=dot(texture(uInputTex,vUV+vec2(-ts.x,-ts.y)).rgb,vec3(0.3,0.59,0.11));
float mt=dot(texture(uInputTex,vUV+vec2(0.0,ts.y)).rgb,vec3(0.3,0.59,0.11));
float mb=dot(texture(uInputTex,vUV+vec2(0.0,-ts.y)).rgb,vec3(0.3,0.59,0.11));
float tr=dot(texture(uInputTex,vUV+vec2(ts.x,ts.y)).rgb,vec3(0.3,0.59,0.11));
float mr=dot(texture(uInputTex,vUV+vec2(ts.x,0.0)).rgb,vec3(0.3,0.59,0.11));
float br=dot(texture(uInputTex,vUV+vec2(ts.x,-ts.y)).rgb,vec3(0.3,0.59,0.11));
float gx=-tl-2.0*ml-bl+tr+2.0*mr+br;
float gy=-tl-2.0*mt-tr+bl+2.0*mb+br;
float g=sqrt(gx*gx+gy*gy);
vec4 sc=texture(uInputTex,vUV);
vec4 bg=mix(sc,vec4(uParamFloat2,uParamFloat3,uParamFloat4,1.0),uParamFloat5);
float edge=clamp(g*8.0,0.0,1.0);
outColor=mix(bg,vec4(vec3(edge),1.0),edge);
}

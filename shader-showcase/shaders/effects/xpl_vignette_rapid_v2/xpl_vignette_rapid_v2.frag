#version 460
// Rapid Vignette V2 + time
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
float I(vec4 c){return sqrt(dot(c.rgb,c.rgb));}
float Sobel(vec2 s,vec2 uv){float tl=I(texture(uInputTex,uv+vec2(-s.x,s.y))),ml=I(texture(uInputTex,uv+vec2(-s.x,0))),bl=I(texture(uInputTex,uv+vec2(-s.x,-s.y))),mt=I(texture(uInputTex,uv+vec2(0,s.y))),mb=I(texture(uInputTex,uv+vec2(0,-s.y))),tr=I(texture(uInputTex,uv+vec2(s.x,s.y))),mr=I(texture(uInputTex,uv+vec2(s.x,0))),br=I(texture(uInputTex,uv+vec2(s.x,-s.y)));return sqrt((tl+2.0*ml+bl-tr-2.0*mr-br)*(tl+2.0*ml+bl-tr-2.0*mr-br)+(-tl-2.0*mt-tr+bl+2.0*mb+br)*(-tl-2.0*mt-tr+bl+2.0*mb+br));}
float Scharr(vec2 s,vec2 uv){float tl=I(texture(uInputTex,uv+vec2(-s.x,s.y))),ml=I(texture(uInputTex,uv+vec2(-s.x,0))),bl=I(texture(uInputTex,uv+vec2(-s.x,-s.y))),mt=I(texture(uInputTex,uv+vec2(0,s.y))),mb=I(texture(uInputTex,uv+vec2(0,-s.y))),tr=I(texture(uInputTex,uv+vec2(s.x,s.y))),mr=I(texture(uInputTex,uv+vec2(s.x,0))),br=I(texture(uInputTex,uv+vec2(s.x,-s.y)));return sqrt((3.0*tl+10.0*ml+3.0*bl-3.0*tr-10.0*mr-3.0*br)*(3.0*tl+10.0*ml+3.0*bl-3.0*tr-10.0*mr-3.0*br)+(-3.0*tl-10.0*mt-3.0*tr+3.0*bl+10.0*mb+3.0*br)*(-3.0*tl-10.0*mt-3.0*tr+3.0*bl+10.0*mb+3.0*br));}
void main() {
vec2 q=vUV-vec2(uParamFloat1,uParamFloat2)-0.03*vec2(cos(uTime*0.35),sin(uTime*0.45));
float d=dot(q,q);float v=1.0-pow(d,clamp(uParamFloat0*0.5,0.25,5.0));v=clamp(v,0,1);
vec3 tint=vec3(uParamFloat3,uParamFloat4,uParamFloat5);
outColor=vec4(mix(tint,texture(uInputTex,vUV).rgb,v),1.0);
}
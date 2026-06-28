#version 460
// Hexagon Pixelate — honeycomb with bold borders, clamped to image edges
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
    float s=max(uParamFloat0,1.0);
    float h=0.8660254*s;
    vec2 pix=vUV*uResolution;
    float rh=floor(pix.y/h);
    float off=mod(rh,2.0)*s*0.5;
    float rd=floor((pix.x-off)/s);
    vec2 center=vec2(rd*s+off+s*0.5, rh*h+h*0.5);
    // Clamp center to image bounds
    center=clamp(center,vec2(0),uResolution);
    vec2 uv2=center/uResolution;
    vec2 d=abs(pix-center);
    float edgeDist=max(abs(d.x/s-d.y/h),abs(d.x/s+d.y/h));
    vec4 col=texture(uInputTex,uv2);
    // Bold hex border, only inside valid cells (not outside image)
    float border=smoothstep(0.65,0.75,edgeDist)-smoothstep(0.78,0.85,edgeDist);
    border=clamp(border,0,1)*clamp(uParamFloat1,0,2);
    outColor=mix(col,vec4(0,0,0,1),border);
}

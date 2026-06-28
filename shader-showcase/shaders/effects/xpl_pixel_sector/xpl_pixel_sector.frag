#version 460
// Sector Pixelate — radial wedges with visible sector boundaries
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
float ps=1.0/max(uParamFloat0,1.0);vec2 q=floor(vUV/ps)*ps;
float a=atan((vUV.y-q.y-ps*0.5),(vUV.x-q.x-ps*0.5))+3.14159;
int numSectors=int(clamp(uParamFloat1,1,16));
float sectorAngle=2.0*3.14159/float(numSectors);
int sec=int(floor(a/sectorAngle));
// Sample from a point slightly rotated within the sector for visual variation
float ca=float(sec)*sectorAngle+sectorAngle*0.5;
vec2 cc=q+ps*0.5+vec2(cos(ca),sin(ca))*ps*0.1;
// Draw sector boundary lines
float angleMod=mod(a,sectorAngle);
float edgeDist=min(angleMod,sectorAngle-angleMod)/sectorAngle;
if(edgeDist<0.05){
    // Dark sector boundary
    vec4 col=texture(uInputTex,cc);
    outColor=mix(vec4(0,0,0,1),col,edgeDist/0.05);
}else{
    outColor=texture(uInputTex,cc);
}
}

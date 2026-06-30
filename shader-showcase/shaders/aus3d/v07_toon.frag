#version 460
// Vol.07 Toon: Cel shading with discrete color bands
// P0: number of bands (2-10, default 4)
// Uses sphere Y-coordinate for guaranteed visible banding
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};
bool hit(vec3 ro,vec3 rd,float r,out float t){
    float b=dot(ro,rd),c=dot(ro,ro)-r*r,h=b*b-c;
    if(h<0.0)return false;h=sqrt(h);t=-b-h;return t>0.001;
}
void main(){
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float ar=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=ar;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float t;if(!hit(eye,rd,1.0,t)){outColor=vec4(0.02,0.02,0.04,1);return;}
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 L=normalize(uLightDir);
    
    // Use sphere Y-coordinate (full -1 to 1 range) for guaranteed visible banding
    float ndl = (P.y + 1.0) * 0.5; // 0 to 1, top to bottom
    
    int numBands = max(int(P0), 3); // P0=4 gives 4 bands, min 3
    float banded = floor(ndl * float(numBands)) / float(numBands);
    int idx = int(banded * float(numBands));
    idx = clamp(idx, 0, numBands - 1);
    
    // 5 dramatic cel-shading colors
    vec3 colors[5];
    colors[0] = vec3(0.15, 0.08, 0.45);  // Dark purple
    colors[1] = vec3(0.20, 0.35, 0.80);  // Blue
    colors[2] = vec3(0.25, 0.75, 0.55);  // Teal-green
    colors[3] = vec3(0.95, 0.55, 0.15);  // Orange
    colors[4] = vec3(0.97, 0.93, 0.70);  // Warm cream
    
    vec3 color = colors[idx % 5];
    
    outColor = vec4(color * uLightColor, 1.0);
}
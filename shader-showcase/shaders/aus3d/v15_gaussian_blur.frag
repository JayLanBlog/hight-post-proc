#version 460
// Vol.15 Gaussian Blur: 7-tap separable gaussian blur on 3D sphere
// P0: blur radius (0-1)
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};
bool hit(vec3 ro,vec3 rd,float r,out float t){
    float b=dot(ro,rd),c=dot(ro,ro)-r*r,h=b*b-c;
    if(h<0.0)return false;h=sqrt(h);t=-b-h;return t>0.001;
}
vec3 sphereColor(vec3 ro,vec3 rd){
    float t;if(!hit(ro,rd,1.0,t))return vec3(0.04,0.04,0.06);
    vec3 P=ro+rd*t;vec3 N=normalize(P);vec3 L=normalize(uLightDir);
    float ndl=dot(N,L)*0.5+0.5;
    return vec3(0.9,0.4,0.3)*ndl;
}
const float weights[7]={0.0205,0.0855,0.232,0.324,0.232,0.0855,0.0205};
void main(){
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float ar=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=ar;

    // Larger blur radius: P0 * 100.0 for dramatic blur
    float stepSize=1.0/float(int(uRes.y))*P0*100.0;

    vec3 col=vec3(0);
    float totalWeight=0.0;

    // Horizontal blur
    for(int i=0;i<7;i++){
        float offset=float(i-3)*stepSize;
        vec3 rdi=normalize(fwd+(uv.x+offset)*rt*0.55+uv.y*up*0.55);
        col+=sphereColor(eye,rdi)*weights[i];
        totalWeight+=weights[i];
    }

    // Vertical blur (combined)
    vec3 col2=vec3(0);
    float totalWeight2=0.0;
    for(int i=0;i<7;i++){
        float offset=float(i-3)*stepSize;
        vec3 rdi=normalize(fwd+uv.x*rt*0.55+(uv.y+offset)*up*0.55);
        col2+=sphereColor(eye,rdi)*weights[i];
        totalWeight2+=weights[i];
    }

    col = (col/totalWeight + col2/totalWeight2) * 0.5;
    outColor = vec4(col * uLightColor, 1);
}
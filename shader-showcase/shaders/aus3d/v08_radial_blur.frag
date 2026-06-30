#version 460
// Vol.08 RadialBlur: 2D gaussian blur on 3D sphere via multi-ray sampling
// P0: blur strength (0-1), P1: unused (grid size fixed for performance)
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
    vec3 centerRd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    vec3 col=vec3(0);float w=0.0;
    float radius=P0*2.0;
    const int N=10; // 10x10 = 100 samples

    for(int j=0;j<N;j++){
        float offY=(float(j)/float(N-1)-0.5)*2.0*radius;
        float wy=exp(-offY*offY*4.0);
        for(int i=0;i<N;i++){
            float offX=(float(i)/float(N-1)-0.5)*2.0*radius;
            float wi=exp(-offX*offX*4.0)*wy;
            vec3 rd=normalize(centerRd+offX*rt*0.5+offY*up*0.5);
            float t;if(hit(eye,rd,1.0,t)){
                vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 L=normalize(uLightDir);
                float ndl=dot(N,L)*0.5+0.5;
                col+=vec3(0.9,0.4,0.3)*ndl*uLightColor*wi;
                w+=wi;
            }
        }
    }
    if(w>0.0){col/=w;outColor=vec4(col,1);}
    else{outColor=vec4(0.02,0.02,0.04,1);}
}
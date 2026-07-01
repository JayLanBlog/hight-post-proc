#version 460
// Vol.03-11 Full Combo: 顶点光照 + 自发光混合 + 纹理混合 + DOUBLE
// Reference: Volume 03 5.顶点光照+自发光混合+纹理混合
// Full formula: lerp(primary, _IlluminCol, texA) * texRGB * blendRGB * primary * 2 + specular
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(binding=2) uniform sampler2D uAuxTex; // 混合纹理
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
    float a=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=a;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float t;if(!hit(eye,rd,1.0,t)){outColor=vec4(0.02,0.02,0.04,1);return;}
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 L=normalize(uLightDir);vec3 V=normalize(eye-P);
    vec2 suv=vec2(atan(P.z,P.x)*0.1591549+0.5,acos(clamp(P.y,-1.0,1.0))*0.3183099);
    // Reference: 两张纹理采样
    vec4 texSample = texture(uInputTex, suv);
    vec3 tex1 = texSample.rgb;
    float texA = texSample.a; // alpha channel for lerp
    vec3 tex2 = texture(uAuxTex, suv).rgb;
    // Vertex lighting: primary = ambient + diffuse (SeparateSpecular: specular added after)
    float ndl = max(dot(N, L), 0.0);
    vec3 ambient = vec3(0.15);
    vec3 diffuse = uLightColor * ndl;
    vec3 primary = ambient + diffuse;
    // Step 1: lerp(vertexLight, _IlluminCol, texA) — constant lerp(texture) previous
    vec3 illuminColor = vec3(P0, P1, P2); // _IlluminCol, default (0,0,0,0)
    vec3 color = mix(primary, illuminColor, texA);
    // Step 2: multiply by main texture RGB — previous * texture
    color = color * tex1;
    // Step 3: multiply by blend texture — previous * texture
    color = color * tex2;
    // Step 4: multiply by primary DOUBLE — previous * primary DOUBLE
    color = color * primary * 2.0;
    // Step 5: SeparateSpecular — specular added after texture combine
    // Shininess[_Shininess] default 0.7 → specPower = 2^(0.7*10) = 128
    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(R, V), 0.0), 128.0);
    vec3 specular = uLightColor * spec;
    color += specular;
    outColor = vec4(color, 1.0);
}
#version 460
// mesh3d.vert — vertex pulling via gl_VertexIndex + SSBO (no vertex input attributes)
layout(std140, binding=1) uniform Params {
    float u0,f1,f2,f3,f4,f5;
    vec2 uResolution; float uTime; float uFrameCount;
    mat4 uMVP;
    mat4 uModelView;
    vec3 uLightDir;  float _p0;
    vec3 uLightColor; float _p1;
    vec3 uEyePos;     float _p2;
};
layout(std430, binding=2) readonly buffer Vertices { float v[]; };

layout(location=0) out vec3 vWorldPos;
layout(location=1) out vec3 vWorldNormal;
layout(location=2) out vec2 vUV;
layout(location=3) out vec3 vEyeDir;
layout(location=4) out vec3 vLightDir;

void main() {
    int i = gl_VertexIndex * 8;
    vec3 pos = vec3(v[i], v[i+1], v[i+2]);
    vec3 nrm = vec3(v[i+3], v[i+4], v[i+5]);
    vec2 uv  = vec2(v[i+6], v[i+7]);

    gl_Position = uMVP * vec4(pos, 1.0);
    vWorldPos   = pos;
    vWorldNormal = normalize(nrm);
    vUV          = uv;
    vEyeDir      = normalize(uEyePos - pos);
    vLightDir    = normalize(uLightDir);
}

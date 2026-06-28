#version 460
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;
layout(location=0) out vec2 vUV;
void main() {
    gl_Position = vec4(aPos.xy * 1.5, 0.5, 1.0);
    vUV = aUV;
}

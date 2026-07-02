#version 450
// particle_paper.frag — burning paper particle with dissolve (matches Unity BurningPaper.shadergraph)
// Used for VFXFireBookScene paper debris particles

layout(binding = 0) uniform sampler2D uInputTex;   // paper texture (page content)
layout(binding = 1, std140) uniform Params {
    float uP0; float uP1; float uP2; float uP3;  // uP0=dissolveAmount, uP1=edgeWidth, uP2=noiseScale
    float uP4; float uP5; float uP6; float uP7;
    float uTime; float uFrameCount; float uPad0; float uPad1;
    mat4 uMVP;
    mat4 uModelView;
    vec4 uLightDir;
    vec4 uLightColor;
    vec4 uEyePos;
};
layout(binding = 2) uniform sampler2D uAuxTex;     // dissolve noise texture (512x512 R8)

layout(location = 0) in vec3 vData;       // x=size, y=lifeRatio, z=rotation
layout(location = 1) in vec2 vUV;

layout(location = 0) out vec4 outColor;

void main() {
    // Circular mask for point sprite
    vec2 center = vUV - 0.5;
    float dist = dot(center, center);
    if (dist > 0.25) discard;

    // Dissolve — matches Unity BurningPaper.shadergraph
    // noise sampled from procedural R8 texture
    float noise = texture(uAuxTex, vUV * uP2).r;

    // Step edge
    float edge = step(uP0, noise);

    // Edge glow (smoothstep for fire rim)
    float glow = 1.0 - smoothstep(uP0, uP0 + uP1, noise);

    // Paper base color from texture
    vec3 texColor = texture(uInputTex, vUV).rgb;

    // Unity DissolveColor = HDR (2.0, 0.523, 0.0) — bright orange fire edge
    vec3 fireColor = vec3(2.0, 0.523, 0.0);

    // Mix: paper on surviving side, fire on dissolving edge
    vec3 finalColor = mix(texColor, fireColor, glow);

    // Alpha: step-based (survive or dissolve), with smooth falloff at edges
    float alpha = edge;
    // Life-based fade (particles fade out as they age)
    alpha *= smoothstep(0.0, 0.2, vData.y) * smoothstep(0.0, 0.3, 1.0 - vData.y);

    // Unity AlphaClip threshold equivalent
    if (alpha < 0.1) discard;

    outColor = vec4(finalColor, alpha);
}

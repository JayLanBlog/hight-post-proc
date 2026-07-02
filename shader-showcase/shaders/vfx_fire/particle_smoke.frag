#version 450
// particle_smoke.frag — smoke particle with 6×6 sequence-frame animation
// Matches Unity SmokeVFX (RealisticSmoke02_6x6.png, 160 spawn/s, additive blend)

layout(binding = 0) uniform sampler2D uInputTex;   // RealisticSmoke02_6x6.png (6x6 grid)
layout(binding = 1, std140) uniform Params {
    float uP0; float uP1; float uP2; float uP3;  // uP0=smokeDensity, uP1 unused, uP2 unused
    float uP4; float uP5; float uP6; float uP7;
    float uTime; float uFrameCount; float uPad0; float uPad1;
    mat4 uMVP;
    mat4 uModelView;
    vec4 uLightDir;
    vec4 uLightColor;
    vec4 uEyePos;
};

layout(location = 0) in vec3 vData;       // x=size, y=lifeRatio, z=rotation
layout(location = 1) in vec2 vUV;

layout(location = 0) out vec4 outColor;

void main() {
    // Circular mask
    vec2 center = vUV - 0.5;
    float dist = dot(center, center);
    if (dist > 0.25) discard;

    // Sequence frame: 6×6 = 36 frames, advance 1 frame every 2 logical frames
    float frame = mod(floor(uFrameCount * 0.5), 36.0);
    float col = mod(frame, 6.0) / 6.0;
    float row = floor(frame / 6.0) / 6.0;
    vec2 frameUV = vUV / 6.0 + vec2(col, row);

    vec4 smoke = texture(uInputTex, frameUV);

    // Black smoke tint (matches Unity SmokeColor override = (0,0,0,1))
    smoke.rgb *= 0.15; // dark gray smoke

    // Density and life-based fade
    smoke.a *= uP0; // uP0 = smokeDensity (0~1)
    smoke.a *= smoothstep(0.0, 0.15, vData.y) * smoothstep(0.0, 0.4, 1.0 - vData.y);

    if (smoke.a < 0.01) discard;

    outColor = smoke;
}

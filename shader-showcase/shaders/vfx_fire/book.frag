#version 450
// book.frag — Blinn-Phong + normal map + diffuse texture
// Matches Unity VFX-SHADER-GRAPH-Magic-Fire-Book directional light (white, intensity 2)

layout(binding = 0) uniform sampler2D uInputTex;   // diffuse texture
layout(binding = 1, std140) uniform Params {
    float uP0; float uP1; float uP2; float uP3;
    float uP4; float uP5; float uP6; float uP7;
    float uTime; float uFrameCount; float uPad0; float uPad1;
    mat4 uMVP;
    mat4 uModelView;
    vec4 uLightDir;    // view-space light direction (xyz)
    vec4 uLightColor;  // (2,2,2) = white×intensity2
    vec4 uEyePos;      // view-space eye (0,0,0)
};
layout(binding = 2) uniform sampler2D uAuxTex;     // normal map

layout(location = 0) in vec3 vViewPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUV;

layout(location = 0) out vec4 outColor;

vec3 perturbNormal(vec3 viewPos, vec3 viewNormal, vec2 uv, vec3 dp1, vec3 dp2) {
    // Sample normal map (tangent-space)
    vec3 tn = texture(uAuxTex, uv).rgb * 2.0 - 1.0;
    tn = normalize(tn);

    // Build TBN from derivatives (no precomputed tangent needed)
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);
    vec3 N = normalize(viewNormal);

    // Gram-Schmidt orthonormalization
    vec3 T = normalize(dp1 * duv2.t - dp2 * duv1.t);
    vec3 B = normalize(cross(N, T));
    T = normalize(cross(B, N)); // re-orthogonalize

    // Perturb normal
    return normalize(T * tn.x + B * tn.y + N * tn.z);
}

void main() {
    vec3 texColor = texture(uInputTex, vUV).rgb;

    // Derivative-based TBN (requires non-uniform control flow guard)
    vec3 dp1 = dFdx(vViewPos);
    vec3 dp2 = dFdy(vViewPos);

    vec3 N = perturbNormal(vViewPos, vNormal, vUV, dp1, dp2);

    // Light direction (view-space, from BookMeshRenderer)
    vec3 L = normalize(-uLightDir.xyz); // lightDir points FROM light, negate for surface->light

    // Eye direction (view-space origin)
    vec3 V = normalize(-vViewPos);

    // Blinn-Phong half vector
    vec3 H = normalize(L + V);

    // Ambient — matches Unity dark-bluish ambient ≈ (0.01,0.01,0.02)
    vec3 ambient = vec3(0.02, 0.02, 0.04) * texColor;

    // Diffuse
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = NdotL * uLightColor.rgb * texColor;

    // Specular (Blinn-Phong, exponent 32 = wider highlight for old book material)
    float NdotH = max(dot(N, H), 0.0);
    vec3 specular = pow(NdotH, 32.0) * uLightColor.rgb * 0.4;

    outColor = vec4(ambient + diffuse + specular, 1.0);
}

#version 460
// aus3d/v01_rim_bump.frag — Ray-marched sphere + AUS Vol.01 Rim+Bump
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5;
    vec2 uResolution; float uTime; float uFrameCount;
    mat4 uMVP; mat4 uModelView;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

// Ray-sphere intersection: returns true and t-min distance if hit
bool hitSphere(vec3 ro, vec3 rd, float r, out float t) {
    float b = dot(ro, rd);
    float c = dot(ro,ro) - r*r;
    float h = b*b - c;
    if (h < 0.0) return false;
    h = sqrt(h);
    float t1 = -b - h, t2 = -b + h;
    t = t1 > 0.001 ? t1 : (t2 > 0.001 ? t2 : -1.0);
    return t > 0.0;
}

void main() {
    // Build ray from camera through pixel
    vec3 eye = uEyePos;
    vec3 fwd = normalize(-eye);
    vec3 right = normalize(cross(fwd, vec3(0,1,0)));
    vec3 up = cross(right, fwd);
    float aspect = uResolution.x / uResolution.y;
    vec2 uv = (vUV - 0.5) * 2.0;
    uv.x *= aspect;
    vec3 rd = normalize(fwd + uv.x * right * 0.7 + uv.y * up * 0.7);

    float t;
    vec3 bg = vec3(0.05, 0.05, 0.08);
    if (!hitSphere(eye, rd, 1.0, t)) { outColor = vec4(bg, 1); return; }

    vec3 P = eye + rd * t;
    vec3 N = normalize(P);
    vec3 V = normalize(eye - P);
    vec3 L = normalize(uLightDir);

    // Diffuse texture sample (checkerboard via uInputTex)
    vec2 sphereUV = vec2(atan(P.z, P.x) * 0.1591549 + 0.5, acos(clamp(P.y, -1.0, 1.0)) * 0.3183099);
    vec3 tex = texture(uInputTex, sphereUV).rgb;

    // Rim light
    float rim = 1.0 - abs(dot(N, V));
    rim = pow(rim, P0);  // P0 = edge power
    vec3 rimColor = vec3(P1, P2, P3);

    // Lambert diffuse
    float NdotL = max(dot(N, L), 0.0);

    vec3 col = tex * NdotL * uLightColor + rimColor * rim * 0.7;
    outColor = vec4(col, 1);
}

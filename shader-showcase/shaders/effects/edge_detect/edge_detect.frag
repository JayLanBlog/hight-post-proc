#version 460
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;

layout(binding=0) uniform sampler2D uInputTex;

layout(std140, binding=1) uniform Params {
    float uParamFloat0;
    float uParamFloat1;
    float uParamFloat2;
    float uParamFloat3;
    float uParamFloat4;
    float uParamFloat5;
    vec2 uResolution;
    float uTime;
    float uFrameCount;
};

void main() {
    vec3 color = texture(uInputTex, vUV).rgb;

    float EdgeStrength = uParamFloat0;
    float ShowColor = uParamFloat1;

    // Early exit if no edge detection
    if (EdgeStrength <= 0.0) {
        outColor = vec4(color, 1.0);
        return;
    }

    vec2 texelSize = 1.0 / uResolution;
    vec3 lumaCoeff = vec3(0.2126, 0.7152, 0.0722);

    // Sample the 3x3 neighborhood
    float tl = dot(texture(uInputTex, vUV + vec2(-1.0, -1.0) * texelSize).rgb, lumaCoeff);
    float t  = dot(texture(uInputTex, vUV + vec2( 0.0, -1.0) * texelSize).rgb, lumaCoeff);
    float tr = dot(texture(uInputTex, vUV + vec2( 1.0, -1.0) * texelSize).rgb, lumaCoeff);
    float l  = dot(texture(uInputTex, vUV + vec2(-1.0,  0.0) * texelSize).rgb, lumaCoeff);
    float r  = dot(texture(uInputTex, vUV + vec2( 1.0,  0.0) * texelSize).rgb, lumaCoeff);
    float bl = dot(texture(uInputTex, vUV + vec2(-1.0,  1.0) * texelSize).rgb, lumaCoeff);
    float b  = dot(texture(uInputTex, vUV + vec2( 0.0,  1.0) * texelSize).rgb, lumaCoeff);
    float br = dot(texture(uInputTex, vUV + vec2( 1.0,  1.0) * texelSize).rgb, lumaCoeff);

    // Sobel X and Y
    float sx = tl * -1.0 + tr * 1.0 + l * -2.0 + r * 2.0 + bl * -1.0 + br * 1.0;
    float sy = tl * -1.0 + t * -2.0 + tr * -1.0 + bl * 1.0 + b * 2.0 + br * 1.0;

    float edge = sqrt(sx * sx + sy * sy) * EdgeStrength;

    // Mix between grayscale edge and color edge
    vec3 edgeColor = mix(vec3(edge), color * edge, ShowColor);

    outColor = vec4(edgeColor, 1.0);
}

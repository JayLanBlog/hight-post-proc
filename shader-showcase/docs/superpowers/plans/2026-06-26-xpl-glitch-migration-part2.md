# XPL Glitch Migration — Part 2: Tasks 11-17 + Integration

> Continuation of part1.md. Contains effects 11-17 (medium/high complexity) and all integration steps.

---

### Task 11: GlitchDigitalStripe (5 params) ⬛ MEDIUM
**Files:** `shaders/effects/xpl_glitch_digital_stripe/` x2
**Note:** XPL uses CPU-generated noise texture; replaced with procedural hash in GLSL.

`.frag`:
```glsl
#version 450
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(push_constant)uniform PC{float uTime,uFrameCount,uIn,uSI,uCR,uCG,uCB;};
float R(vec2 s){return fract(sin(dot(s,vec2(12.9898,78.233)))*43758.5453);}
void main(){
    vec4 sn=vec4(R(vUV*200.0+vec2(uTime*0.1)),R(vUV*200.0+vec2(0.3,0.7)+uTime*0.13),
                 R(vUV*200.0+vec2(0.7,0.3)+uTime*0.17),R(vUV*200.0+vec2(0.5,0.5)+uTime*0.11));
    float t=1.001-uIn*1.001;
    float us=step(t,pow(abs(sn.x),3.0));
    vec2 uv=fract(vUV+sn.yz*us);
    vec4 src=texture(uInputTex,uv);
    vec3 sc=vec3(uCR,uCG,uCB);
    float si=step(t,pow(abs(sn.w),3.0))*uSI;
    vec3 col=mix(src.rgb,sc,si);
    fC=vec4(col,src.a);
}
```

`effect.json`:
```json
{"name":"Glitch Digital Stripe","name_cn":"数字条纹","category":"Glitch Effects","category_cn":"故障效果","description":"Digital stripe glitch with noise-driven UV shifting and trash frame color overlay.","description_cn":"数字条纹故障——噪声UV偏移+残帧色彩叠加。","params":[{"name":"uParamFloat0","label":"Intensity","label_cn":"强度","type":"float","min":0,"max":1,"default":0.25,"ui":"slider"},{"name":"uParamFloat1","label":"Strip Color Intensity","label_cn":"条纹色强度","type":"float","min":0,"max":10,"default":2,"ui":"slider"},{"name":"uParamFloat2","label":"Strip Color R","label_cn":"条纹色R","type":"float","min":0,"max":1,"default":0.1,"ui":"slider"},{"name":"uParamFloat3","label":"Strip Color G","label_cn":"条纹色G","type":"float","min":0,"max":1,"default":0.1,"ui":"slider"},{"name":"uParamFloat4","label":"Strip Color B","label_cn":"条纹色B","type":"float","min":0,"max":1,"default":0.1,"ui":"slider"}]}
```

### Task 12: GlitchImageBlockV2 (7 params) ⬛ MEDIUM
**Files:** `shaders/effects/xpl_glitch_image_block_v2/` x2

`.frag`:
```glsl
#version 450
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(push_constant)uniform PC{float uTime,uFrameCount,uSp,uOff,uBU,uBV,uBI,uRS,uFd;};
float R(vec2 s){return fract(sin(dot(s*floor(uTime*uSp*30.0),vec2(127.1,311.7)))*43758.5453123);}
float Rf(float s){return R(vec2(s,1.0));}
void main(){
    vec2 bl=floor(vUV*vec2(uBU,uBV));
    float ln=pow(R(bl),uBI)*uOff-pow(Rf(5.1379),7.1)*uRS;
    float r=texture(uInputTex,vUV).r;
    float g=texture(uInputTex,vUV+vec2(ln*0.05*Rf(5.0),0.0)).g;
    float b=texture(uInputTex,vUV-vec2(ln*0.05*Rf(31.0),0.0)).b;
    vec4 res=vec4(r,g,b,1.0);
    vec4 orig=texture(uInputTex,vUV);
    fC=mix(orig,res,uFd);
}
```

`effect.json`:
```json
{"name":"Glitch Image Block V2","name_cn":"画面块错位 V2","category":"Glitch Effects","category_cn":"故障效果","description":"Single block layer displacement with RGB split and fade control.","description_cn":"单层方块错位+RGB分裂+渐变控制。","params":[{"name":"uParamFloat0","label":"Speed","label_cn":"速度","type":"float","min":0,"max":1,"default":0.5,"ui":"slider"},{"name":"uParamFloat1","label":"Offset Amount","label_cn":"偏移量","type":"float","min":0,"max":10,"default":1,"ui":"slider"},{"name":"uParamFloat2","label":"Block U","label_cn":"块列数","type":"float","min":1,"max":50,"default":2,"ui":"slider"},{"name":"uParamFloat3","label":"Block V","label_cn":"块行数","type":"float","min":1,"max":50,"default":16,"ui":"slider"},{"name":"uParamFloat4","label":"Block Intensity","label_cn":"块强度","type":"float","min":1,"max":50,"default":8,"ui":"slider"},{"name":"uParamFloat5","label":"RGB Split","label_cn":"RGB分裂","type":"float","min":0,"max":50,"default":2,"ui":"slider"},{"name":"uParamFloat6","label":"Fade","label_cn":"渐变","type":"float","min":0,"max":1,"default":1,"ui":"slider"}]}
```

### Task 13: GlitchImageBlock V1 (10 params) ⬛ MEDIUM
**Files:** `shaders/effects/xpl_glitch_image_block_v1/` x2

`.frag`:
```glsl
#version 450
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(push_constant)uniform PC{float uTime,uFrameCount,uSp,uOff; float u1U,u1V,u2U,u2V; float u1I,u2I,uRS,uFd;};
float R(vec2 s){return fract(sin(dot(s*floor(uTime*uSp*30.0),vec2(127.1,311.7)))*43758.5453123);}
float Rf(float s){return R(vec2(s,1.0));}
void main(){
    vec2 b1=floor(vUV*vec2(u1U,u1V));vec2 b2=floor(vUV*vec2(u2U,u2V));
    float ln1=pow(R(b1),u1I);float ln2=pow(R(b2),u2I);
    float rn=pow(Rf(5.1379),7.1)*uRS;
    float ln=ln1*ln2*uOff-rn;
    float r=texture(uInputTex,vUV).r;
    float g=texture(uInputTex,vUV+vec2(ln*0.05*Rf(7.0),0.0)).g;
    float b=texture(uInputTex,vUV-vec2(ln*0.05*Rf(23.0),0.0)).b;
    vec4 res=vec4(r,g,b,1.0);vec4 orig=texture(uInputTex,vUV);
    fC=mix(orig,res,uFd);
}
```

`effect.json`:
```json
{"name":"Glitch Image Block V1","name_cn":"画面块错位 V1","category":"Glitch Effects","category_cn":"故障效果","description":"Dual block layer displacement with RGB split. Two grid layers for complex block corruption.","description_cn":"双层方块错位+RGB分离——双网格产生复杂块损坏效果。","params":[{"name":"uParamFloat0","label":"Speed","label_cn":"速度","type":"float","min":0,"max":1,"default":0.5,"ui":"slider"},{"name":"uParamFloat1","label":"Offset Amount","label_cn":"偏移量","type":"float","min":0,"max":10,"default":1,"ui":"slider"},{"name":"uParamFloat2","label":"Block1 Columns","label_cn":"层1列数","type":"float","min":1,"max":50,"default":9,"ui":"slider"},{"name":"uParamFloat3","label":"Block1 Rows","label_cn":"层1行数","type":"float","min":1,"max":50,"default":9,"ui":"slider"},{"name":"uParamFloat4","label":"Block2 Columns","label_cn":"层2列数","type":"float","min":1,"max":50,"default":5,"ui":"slider"},{"name":"uParamFloat5","label":"Block2 Rows","label_cn":"层2行数","type":"float","min":1,"max":50,"default":5,"ui":"slider"},{"name":"uParamFloat6","label":"Block1 Intensity","label_cn":"层1强度","type":"float","min":1,"max":50,"default":8,"ui":"slider"},{"name":"uParamFloat7","label":"Block2 Intensity","label_cn":"层2强度","type":"float","min":1,"max":50,"default":4,"ui":"slider"},{"name":"uParamFloat8","label":"RGB Split","label_cn":"RGB分裂","type":"float","min":0,"max":50,"default":0.5,"ui":"slider"},{"name":"uParamFloat9","label":"Fade","label_cn":"渐变","type":"float","min":0,"max":1,"default":1,"ui":"slider"}]}
```

### Task 14: GlitchRGBSplit V1 (6 params + direction) ⬛ MEDIUM
**Files:** `shaders/effects/xpl_glitch_rgb_split_v1/` x2

`.frag`:
```glsl
#version 450
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(push_constant)uniform PC{float uTime,uFrameCount,uFd,uAm,uSp,uAR,uAB,uCF,uD;};
void main(){
    vec2 uv=vUV;float t=uTime*6.0*uSp;
    float sa=(1.0+sin(t))*0.5;sa*=1.0+sin(t*2.0)*0.5;
    sa=pow(sa,3.0)*0.05;
    float d=length(uv-vec2(0.5));
    sa*=uFd*uAm;sa*=mix(1.0,d,uCF);
    vec3 sc=texture(uInputTex,uv).rgb;
    if(uD<0.5){//H
        float r=texture(uInputTex,vec2(uv.x+sa*uAR,uv.y)).r;
        float b=texture(uInputTex,vec2(uv.x-sa*uAB,uv.y)).b;
        fC=vec4(mix(sc,vec3(r,sc.g,b),uFd),1.0);
    }else if(uD<1.5){//V
        float r=texture(uInputTex,vec2(uv.x,uv.y+sa*uAR)).r;
        float b=texture(uInputTex,vec2(uv.x,uv.y-sa*uAB)).b;
        fC=vec4(mix(sc,vec3(r,sc.g,b),uFd),1.0);
    }else{//H+V
        float r=texture(uInputTex,vec2(uv.x+sa*uAR,uv.y+sa*uAR)).r;
        float b=texture(uInputTex,vec2(uv.x-sa*uAB,uv.y-sa*uAB)).b;
        fC=vec4(mix(sc,vec3(r,sc.g,b),uFd),1.0);
    }
}
```

`effect.json`:
```json
{"name":"Glitch RGB Split V1","name_cn":"RGB分离 V1","category":"Glitch Effects","category_cn":"故障效果","description":"Classic RGB split with sine pulsing, center-distance fading, per-channel amount.","description_cn":"经典RGB分离——正弦脉冲+距中心衰减+每通道独立量。","params":[{"name":"uParamFloat0","label":"Fading","label_cn":"渐变","type":"float","min":0,"max":1,"default":1,"ui":"slider"},{"name":"uParamFloat1","label":"Amount","label_cn":"强度","type":"float","min":0,"max":5,"default":1,"ui":"slider"},{"name":"uParamFloat2","label":"Speed","label_cn":"速度","type":"float","min":0,"max":10,"default":1,"ui":"slider"},{"name":"uParamFloat3","label":"Amount R","label_cn":"R通道量","type":"float","min":0,"max":5,"default":1,"ui":"slider"},{"name":"uParamFloat4","label":"Amount B","label_cn":"B通道量","type":"float","min":0,"max":5,"default":1,"ui":"slider"},{"name":"uParamFloat5","label":"Center Fading","label_cn":"中心衰减","type":"float","min":0,"max":1,"default":1,"ui":"slider"}]}
```

### Task 15: GlitchRGBSplitV5 (2 params) ⬛ MEDIUM
**Files:** `shaders/effects/xpl_glitch_rgb_split_v5/` x2
**Note:** XPL uses `Resources.Load("X-Noise256")` texture; replaced with procedural 4-channel hash.

`.frag`:
```glsl
#version 450
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(push_constant)uniform PC{float uTime,uFrameCount,uAp,uSp;};
float R(vec2 s){return fract(sin(dot(s,vec2(12.9898,78.233)))*43758.5453);}
vec4 N(vec2 p){return vec4(R(p),R(p+vec2(0.33,0.67)),R(p+vec2(0.67,0.33)),R(p+vec2(0.5,0.5)));}
void main(){
    vec2 nu=vec2(uSp*uTime,2.0*uSp*uTime/25.0);
    vec4 sa=vec4(pow(N(nu).x,8.0),pow(N(nu+vec2(0.1,0.2)).y,8.0),pow(N(nu+vec2(0.3,0.4)).z,8.0),1.0)*uAp;
    sa*=2.0*sa.w-1.0;
    float r=texture(uInputTex,vUV+vec2(sa.x,-sa.y)).r;
    float g=texture(uInputTex,vUV+vec2(sa.y,-sa.z)).g;
    float b=texture(uInputTex,vUV+vec2(sa.z,-sa.x)).b;
    fC=vec4(r,g,b,1.0);
}
```

`effect.json`:
```json
{"name":"Glitch RGB Split V5","name_cn":"RGB分离 V5","category":"Glitch Effects","category_cn":"故障效果","description":"Noise-driven RGB split with cross-channel permutation. Chaotic color displacement.","description_cn":"噪声驱动RGB分离——交叉通道置换产生混沌色彩偏移。","params":[{"name":"uParamFloat0","label":"Amplitude","label_cn":"振幅","type":"float","min":0,"max":5,"default":3,"ui":"slider"},{"name":"uParamFloat1","label":"Speed","label_cn":"速度","type":"float","min":0,"max":1,"default":0.1,"ui":"slider"}]}
```

### Task 16: GlitchLineBlock (6 params) ⬛ HIGH
**Files:** `shaders/effects/xpl_glitch_line_block/` x2
**Note:** Includes YUV↔RGB color space conversion + truncation + multi-step noise generation. Most complex effect.

`.frag`:
```glsl
#version 450
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(push_constant)uniform PC{float uTime,uFrameCount,uFq,uSp,uAm,uOff,uLW,uAl,uD;};
float R(vec2 s){return fract(sin(dot(s,vec2(12.9898,78.233)))*43758.5453);}
float T(float x,float L){return floor(x*L)/L;}
vec2 T2(vec2 x,vec2 L){return floor(x*L)/L;}
vec3 rgb2yuv(vec3 c){return vec3(dot(c,vec3(0.299,0.587,0.114)),dot(c,vec3(-0.14713,-0.28886,0.436)),dot(c,vec3(0.615,-0.51499,-0.10001)));}
vec3 yuv2rgb(vec3 c){return vec3(dot(c,vec3(1.0,0.0,1.13983)),dot(c,vec3(1.0,-0.39465,-0.58060)),dot(c,vec3(1.0,2.03211,0.0)));}
void main(){
    float tX=uTime*uSp*0.2;vec2 uv=vUV;
    float st=0.5+0.5*cos(tX*uFq);tX*=st;
    float tT=T(tX,4.0);
    vec2 ac=uD>0.5?uv.xx:uv.yy;
    float ut=R(T2(ac,vec2(8.0))+100.0*tT);
    float ur=6.0*T(tX,24.0*ut);
    float lr=0.5*R(T2(ac+vec2(ur),vec2(8.0/uLW)))+0.5*R(T2(ac+vec2(ur),vec2(7.0)));
    lr=lr*2.0-1.0;lr=sign(lr)*clamp((abs(lr)-uAm)/0.4,0.0,1.0);lr=mix(0.0,lr,uOff);
    vec2 ubl=uv;if(uD>0.5)ubl=clamp(ubl+vec2(0.0,0.1*lr),0.0,1.0);else ubl=clamp(ubl+vec2(0.1*lr,0.0),0.0,1.0);
    vec4 blC=texture(uInputTex,abs(ubl));
    vec3 yuv=rgb2yuv(blC.rgb);
    yuv.y/=1.0-3.0*abs(lr)*clamp(0.5-lr,0.0,1.0);
    yuv.z+=0.125*lr*clamp(lr-0.5,0.0,1.0);
    vec3 rgb=yuv2rgb(yuv);
    vec4 sc=texture(uInputTex,uv);
    fC=mix(sc,vec4(rgb,blC.a),uAl);
}
```

`effect.json`:
```json
{"name":"Glitch Line Block","name_cn":"行块错位","category":"Glitch Effects","category_cn":"故障效果","description":"Complex rhythmic line block displacement with YUV chrominance distortion. The most intricate glitch effect.","description_cn":"复杂节律行块错位+YUV色度畸变。最复杂的故障效果。","params":[{"name":"uParamFloat0","label":"Frequency","label_cn":"频率","type":"float","min":0,"max":25,"default":1,"ui":"slider"},{"name":"uParamFloat1","label":"Speed","label_cn":"速度","type":"float","min":0,"max":1,"default":0.8,"ui":"slider"},{"name":"uParamFloat2","label":"Amount","label_cn":"强度","type":"float","min":0,"max":1,"default":0.5,"ui":"slider"},{"name":"uParamFloat3","label":"Offset","label_cn":"偏移","type":"float","min":0,"max":13,"default":1,"ui":"slider"},{"name":"uParamFloat4","label":"Lines Width","label_cn":"线宽","type":"float","min":0.1,"max":10,"default":1,"ui":"slider"},{"name":"uParamFloat5","label":"Alpha","label_cn":"透明度","type":"float","min":0,"max":1,"default":1,"ui":"slider"}]}
```

### Task 17: GlitchWaveJitter (4 params) ⬛ HIGH
**Files:** `shaders/effects/xpl_glitch_wave_jitter/` x2
**Note:** Needs full 2D simplex noise implementation inline.

`.frag`:
```glsl
#version 450
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(push_constant)uniform PC{float uTime,uFrameCount,uFq,uRS,uSp,uAm,uD;};

// 2D Simplex noise
vec3 m289(vec3 x){return x-floor(x*(1.0/289.0))*289.0;}
vec2 m289(vec2 x){return x-floor(x*(1.0/289.0))*289.0;}
vec3 pte(vec3 x){return m289(((x*34.0)+10.0)*x);}
float snoise(vec2 v){
    const vec4 C=vec4(0.211324865405187,0.366025403784439,-0.577350269189626,0.024390243902439);
    vec2 i=floor(v+dot(v,C.yy));vec2 x0=v-i+dot(i,C.xx);
    vec2 i1=(x0.x>x0.y)?vec2(1.0,0.0):vec2(0.0,1.0);
    vec4 x12=x0.xyxy+C.xxzz;x12.xy-=i1;i=m289(i);
    vec3 p=pte(pte(i.y+vec3(0.0,i1.y,1.0))+i.x+vec3(0.0,i1.x,1.0));
    vec3 m=max(0.5-vec3(dot(x0,x0),dot(x12.xy,x12.xy),dot(x12.zw,x12.zw)),0.0);
    m=m*m;m=m*m;
    vec3 x=2.0*fract(p*C.www)-1.0;vec3 h=abs(x)-0.5;vec3 ox=floor(x+0.5);
    vec3 a0=x-ox;m*=1.79284291400159-0.85373472095314*(a0*a0+h*h);
    vec3 g;g.x=a0.x*x0.x+h.x*x0.y;g.yz=a0.yz*x12.xz+h.yz*x12.yw;
    return 130.0*dot(m,g);
}

void main(){
    float res=uD>0.5?1080.0:1920.0;
    float st=0.5+0.5*cos(uTime*uFq);
    float uvy=vUV.y*res;
    float n1=snoise(vec2(uvy*0.01,uTime*uSp*20.0))*(st*uAm*32.0);
    float n2=snoise(vec2(uvy*0.02,uTime*uSp*10.0))*(st*uAm*4.0);
    float nwx=n1*n2/res;
    float ux=vUV.x+nwx;
    float rsx=(uRS*50.0+(20.0*st+1.0))*nwx/res;
    vec4 cG=texture(uInputTex,vec2(ux,vUV.y));
    vec4 cRB=texture(uInputTex,vec2(ux+rsx,vUV.y));
    fC=vec4(cRB.r,cG.g,cRB.b,cRB.a+cG.a);
}
```

`effect.json`:
```json
{"name":"Glitch Wave Jitter","name_cn":"波浪抖动","category":"Glitch Effects","category_cn":"故障效果","description":"Simplex-noise-driven wave displacement with RGB split. Resolution-aware per-line distortion.","description_cn":"Simplex噪声驱动的波浪位移+RGB分裂——逐行分辨率感知畸变。","params":[{"name":"uParamFloat0","label":"Frequency","label_cn":"频率","type":"float","min":0,"max":50,"default":5,"ui":"slider"},{"name":"uParamFloat1","label":"RGB Split","label_cn":"RGB分裂","type":"float","min":0,"max":50,"default":20,"ui":"slider"},{"name":"uParamFloat2","label":"Speed","label_cn":"速度","type":"float","min":0,"max":1,"default":0.25,"ui":"slider"},{"name":"uParamFloat3","label":"Amount","label_cn":"强度","type":"float","min":0,"max":2,"default":1,"ui":"slider"}]}
```

---

## Integration Tasks

### Task 18: CMake — Register 17 compile_frag() calls

**Files:** Modify `shaders/CMakeLists.txt` (append after line 90)

- [ ] Append after existing `compile_frag(lens_distort)`:

```cmake
# === XPL Glitch Effects (17) ===
compile_frag(xpl_glitch_screen_jump)
compile_frag(xpl_glitch_screen_shake)
compile_frag(xpl_glitch_scan_line_jitter)
compile_frag(xpl_glitch_rgb_split_v4)
compile_frag(xpl_glitch_rgb_split_v2)
compile_frag(xpl_glitch_analog_noise)
compile_frag(xpl_glitch_image_block_v3)
compile_frag(xpl_glitch_image_block_v4)
compile_frag(xpl_glitch_tile_jitter)
compile_frag(xpl_glitch_rgb_split_v3)
compile_frag(xpl_glitch_digital_stripe)
compile_frag(xpl_glitch_image_block_v2)
compile_frag(xpl_glitch_image_block_v1)
compile_frag(xpl_glitch_rgb_split_v1)
compile_frag(xpl_glitch_rgb_split_v5)
compile_frag(xpl_glitch_line_block)
compile_frag(xpl_glitch_wave_jitter)
```

### Task 19: CoverFlowScene.cpp — Register 17 cards

**Files:** Modify `src/app/CoverFlowScene.cpp` (append before `#undef CARD`)

- [ ] Append 17 CARD() calls before `#undef CARD`:

```cpp
    // === XPL Glitch Effects (17) ===
    CARD("xpl_glitch_screen_jump",      "Glitch Screen Jump",    "Glitch Effects",
         "Screen jump displacement with horizontal/vertical scrolling.",
         "effects/xpl_glitch_screen_jump/xpl_glitch_screen_jump.frag.spv");
    CARD("xpl_glitch_screen_shake",     "Glitch Screen Shake",   "Glitch Effects",
         "Random screen shake simulating handheld camera/earthquake.",
         "effects/xpl_glitch_screen_shake/xpl_glitch_screen_shake.frag.spv");
    CARD("xpl_glitch_scan_line_jitter", "Glitch Scan Line Jitter","Glitch Effects",
         "Horizontal scan line random jitter.",
         "effects/xpl_glitch_scan_line_jitter/xpl_glitch_scan_line_jitter.frag.spv");
    CARD("xpl_glitch_rgb_split_v4",     "Glitch RGB Split V4",   "Glitch Effects",
         "Clean jittery RGB channel separation with time-quantized noise.",
         "effects/xpl_glitch_rgb_split_v4/xpl_glitch_rgb_split_v4.frag.spv");
    CARD("xpl_glitch_rgb_split_v2",     "Glitch RGB Split V2",   "Glitch Effects",
         "Organic pulsing chromatic aberration from 4-sine product.",
         "effects/xpl_glitch_rgb_split_v2/xpl_glitch_rgb_split_v2.frag.spv");
    CARD("xpl_glitch_analog_noise",     "Glitch Analog Noise",   "Glitch Effects",
         "Analog static interference with luminance jitter and color noise.",
         "effects/xpl_glitch_analog_noise/xpl_glitch_analog_noise.frag.spv");
    CARD("xpl_glitch_image_block_v3",   "Glitch Image Block V3", "Glitch Effects",
         "Single-layer block displacement with RGB split.",
         "effects/xpl_glitch_image_block_v3/xpl_glitch_image_block_v3.frag.spv");
    CARD("xpl_glitch_image_block_v4",   "Glitch Image Block V4", "Glitch Effects",
         "Dual-axis block RGB split with per-block channel shifting.",
         "effects/xpl_glitch_image_block_v4/xpl_glitch_image_block_v4.frag.spv");
    CARD("xpl_glitch_tile_jitter",      "Glitch Tile Jitter",    "Glitch Effects",
         "Alternating tile band jitter with frequency gating.",
         "effects/xpl_glitch_tile_jitter/xpl_glitch_tile_jitter.frag.spv");
    CARD("xpl_glitch_rgb_split_v3",     "Glitch RGB Split V3",   "Glitch Effects",
         "Multi-channel sine-wave RGB split with 3 direction modes.",
         "effects/xpl_glitch_rgb_split_v3/xpl_glitch_rgb_split_v3.frag.spv");
    CARD("xpl_glitch_digital_stripe",   "Glitch Digital Stripe", "Glitch Effects",
         "Noise-driven UV shifting with trash frame color overlay.",
         "effects/xpl_glitch_digital_stripe/xpl_glitch_digital_stripe.frag.spv");
    CARD("xpl_glitch_image_block_v2",   "Glitch Image Block V2", "Glitch Effects",
         "Single block layer displacement with fade control.",
         "effects/xpl_glitch_image_block_v2/xpl_glitch_image_block_v2.frag.spv");
    CARD("xpl_glitch_image_block_v1",   "Glitch Image Block V1", "Glitch Effects",
         "Dual block layer grid corruption with RGB split.",
         "effects/xpl_glitch_image_block_v1/xpl_glitch_image_block_v1.frag.spv");
    CARD("xpl_glitch_rgb_split_v1",     "Glitch RGB Split V1",   "Glitch Effects",
         "Classic sine-pulsing RGB split with center-distance fading.",
         "effects/xpl_glitch_rgb_split_v1/xpl_glitch_rgb_split_v1.frag.spv");
    CARD("xpl_glitch_rgb_split_v5",     "Glitch RGB Split V5",   "Glitch Effects",
         "Noise-driven cross-channel permutation RGB split.",
         "effects/xpl_glitch_rgb_split_v5/xpl_glitch_rgb_split_v5.frag.spv");
    CARD("xpl_glitch_line_block",       "Glitch Line Block",     "Glitch Effects",
         "Complex rhythmic line block with YUV chrominance distortion.",
         "effects/xpl_glitch_line_block/xpl_glitch_line_block.frag.spv");
    CARD("xpl_glitch_wave_jitter",      "Glitch Wave Jitter",    "Glitch Effects",
         "Simplex-noise wave displacement with per-line RGB split.",
         "effects/xpl_glitch_wave_jitter/xpl_glitch_wave_jitter.frag.spv");
```

### Task 20: Update Dynamic index lists

**Files:** Modify `src/app/CoverFlowScene.cpp` (the Dynamic/Static builder)

- [ ] Update the Dynamic IDs list to include ALL XPL glitch effects:

Replace the existing Dynamic filter line with:
```cpp
        if (id == "water_ripple" || id == "vhs" || id == "noise" ||
            id == "glitch" || id == "crt" || id == "kaleidoscope" ||
            id == "xpl_glitch_analog_noise" || id == "xpl_glitch_digital_stripe" ||
            id == "xpl_glitch_image_block_v1" || id == "xpl_glitch_image_block_v2" ||
            id == "xpl_glitch_image_block_v3" || id == "xpl_glitch_image_block_v4" ||
            id == "xpl_glitch_line_block" ||
            id == "xpl_glitch_rgb_split_v1" || id == "xpl_glitch_rgb_split_v2" ||
            id == "xpl_glitch_rgb_split_v3" || id == "xpl_glitch_rgb_split_v4" || id == "xpl_glitch_rgb_split_v5" ||
            id == "xpl_glitch_scan_line_jitter" || id == "xpl_glitch_screen_jump" ||
            id == "xpl_glitch_screen_shake" || id == "xpl_glitch_tile_jitter" || id == "xpl_glitch_wave_jitter") {
```

### Task 21: Build and test

**Files:** None (validation only)

- [ ] Run CMake configure: `cmake -B build -DCMAKE_BUILD_TYPE=Release`
- [ ] Run full rebuild: `cmake --build build --config Release`
- [ ] Check for SPIR-V compilation errors in all 17 `.frag` files
- [ ] Run: `.\build\bin\Release\ShaderShowcase.exe`
- [ ] Navigate to Glitch Effects category — verify all 17 new cards appear (total: 1 original + 17 new = 18 Glitch cards)
- [ ] Click each card — verify shader renders without black/crash
- [ ] Toggle Dynamic/Static — verify all 17 XPL glitch cards are in Dynamic pool

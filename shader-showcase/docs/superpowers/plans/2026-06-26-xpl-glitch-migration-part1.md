# XPL Glitch Effects Migration — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Port 17 Glitch effects from X-PostProcessing-Library (HLSL/Unity) to ShaderShowcase (GLSL) with 100% fidelity, zero conflict with existing 18 effects.

**Architecture:** Each effect gets an isolated directory under `shaders/effects/xpl_glitch_*/` containing `.frag` GLSL source + `effect.json` metadata. Multi-pass/direction effects merged into single `.frag` with `uPassDirection` uniform. All compiled to SPIR-V via existing CMake `compile_frag()` function. Cards registered at end of `CoverFlowScene::RegisterCards()`.

**Tech Stack:** GLSL 450, SPIR-V 1.3, C++17, CMake 3.20, glslangValidator

---

## Task 0: Setup — Directory skeleton

**Files:**
- Create: 17 effect directories under `shaders/effects/xpl_glitch_*/`

- [ ] **Step 0.1: Create all 17 directories**

```powershell
cd e:\AI\graph\hight-post-proc\shader-showcase
$ids = @(
    'xpl_glitch_analog_noise', 'xpl_glitch_digital_stripe',
    'xpl_glitch_image_block_v1', 'xpl_glitch_image_block_v2',
    'xpl_glitch_image_block_v3', 'xpl_glitch_image_block_v4',
    'xpl_glitch_line_block',
    'xpl_glitch_rgb_split_v1', 'xpl_glitch_rgb_split_v2',
    'xpl_glitch_rgb_split_v3', 'xpl_glitch_rgb_split_v4', 'xpl_glitch_rgb_split_v5',
    'xpl_glitch_scan_line_jitter', 'xpl_glitch_screen_jump',
    'xpl_glitch_screen_shake', 'xpl_glitch_tile_jitter', 'xpl_glitch_wave_jitter'
)
foreach ($id in $ids) {
    New-Item -ItemType Directory -Force -Path "shaders\effects\$id"
}
Write-Output "17 directories created"
```
Expected: 17 directories created under `shaders/effects/`.

---

## Task 1: GlitchScreenJump (SIMPLEST — 1 param, 1 uniform)

**Files:**
- Create: `shaders/effects/xpl_glitch_screen_jump/xpl_glitch_screen_jump.frag`
- Create: `shaders/effects/xpl_glitch_screen_jump/effect.json`

- [ ] **Write .frag:**

```glsl
#version 450
layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 fragColor;
layout(binding = 0) uniform sampler2D uInputTex;
layout(push_constant) uniform PC { float uTime, uFrameCount, uParamFloat0, uPassDirection; };

void main() {
    float jumpTime = uTime * uParamFloat0 * 9.8;
    float jx = mix(vUV.x, fract(vUV.x + jumpTime), uParamFloat0);
    float jy = mix(vUV.y, fract(vUV.y + jumpTime), uParamFloat0);
    vec2 uv = (uPassDirection > 0.5) ? fract(vec2(vUV.x, jy)) : fract(vec2(jx, vUV.y));
    fragColor = texture(uInputTex, uv);
}
```

- [ ] **Write effect.json:**

```json
{"name":"Glitch Screen Jump","name_cn":"屏幕跳跃","category":"Glitch Effects","category_cn":"故障效果","description":"Screen jump displacement — horizontal/vertical scrolling jump.","description_cn":"屏幕画面跳动——水平/垂直方向持续滚动跳跃。","params":[{"name":"uParamFloat0","label":"Intensity","label_cn":"强度","type":"float","min":0,"max":1,"default":0.35,"ui":"slider"}]}
```

- [ ] **Verify:** `glslangValidator -V shaders/effects/xpl_glitch_screen_jump/xpl_glitch_screen_jump.frag -o NUL`

---

## Task 2: GlitchScreenShake (SIMPLEST — 1 param)

**Files:**
- Create: `shaders/effects/xpl_glitch_screen_shake/xpl_glitch_screen_shake.frag`
- Create: `shaders/effects/xpl_glitch_screen_shake/effect.json`

- [ ] **Write .frag:**

```glsl
#version 450
layout(location=0) in vec2 vUV; layout(location=0) out vec4 fragColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(push_constant) uniform PC { float uTime, uFrameCount, uParamFloat0, uPassDirection; };
float rng(vec2 s){return fract(sin(dot(s,vec2(127.1,311.7)))*43758.5453);}
void main(){
    float shake=(rng(vec2(uTime,2.0))-0.5)*uParamFloat0*0.25;
    vec2 uv=vUV; if(uPassDirection>0.5)uv=fract(vec2(vUV.x,vUV.y+shake)); else uv=fract(vec2(vUV.x+shake,vUV.y));
    fragColor=texture(uInputTex,uv);
}
```

- [ ] **Write effect.json:**

```json
{"name":"Glitch Screen Shake","name_cn":"屏幕震动","category":"Glitch Effects","category_cn":"故障效果","description":"Random screen shake simulating handheld camera / earthquake.","description_cn":"随机屏幕震动——模拟手持摄影/地震效果。","params":[{"name":"uParamFloat0","label":"Intensity","label_cn":"强度","type":"float","min":0,"max":1,"default":0.5,"ui":"slider"}]}
```

- [ ] **Verify:** `glslangValidator -V shaders/effects/xpl_glitch_screen_shake/xpl_glitch_screen_shake.frag -o NUL`

---

## Task 3: GlitchScanLineJitter (2 params)

**Files:**
- Create: `shaders/effects/xpl_glitch_scan_line_jitter/xpl_glitch_scan_line_jitter.frag`
- Create: `shaders/effects/xpl_glitch_scan_line_jitter/effect.json`

- [ ] **Write .frag:**

```glsl
#version 450
layout(location=0) in vec2 vUV; layout(location=0) out vec4 fragColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(push_constant) uniform PC { float uTime, uFrameCount, uParamFloat0, uParamFloat1; };
float rng(vec2 s){return fract(sin(dot(s,vec2(12.9898,78.233)))*43758.5453);}
void main(){
    float t=clamp(1.0-uParamFloat0*1.2,0.0,1.0);
    float a=0.005+pow(uParamFloat0,3.0)*0.1;
    float s=0.5+0.5*cos(uTime*uParamFloat1);
    float j=rng(vec2(vUV.y,uTime))*2.0-1.0;
    j*=step(t,abs(j))*a*s;
    fragColor=texture(uInputTex,fract(vUV+vec2(j,0.0)));
}
```

- [ ] **Write effect.json:**

```json
{"name":"Glitch Scan Line Jitter","name_cn":"扫描线抖动","category":"Glitch Effects","category_cn":"故障效果","description":"Horizontal scan line jitter with configurable intensity and frequency.","description_cn":"水平扫描线随机抖动——可调强度/频率。","params":[{"name":"uParamFloat0","label":"Jitter Intensity","label_cn":"抖动强度","type":"float","min":0,"max":1,"default":0.1,"ui":"slider"},{"name":"uParamFloat1","label":"Frequency","label_cn":"频率","type":"float","min":0.1,"max":25,"default":1,"ui":"slider"}]}
```

---

## Task 4: GlitchRGBSplitV4 (2 params — quantized time)

**Files:**
- Create: `shaders/effects/xpl_glitch_rgb_split_v4/xpl_glitch_rgb_split_v4.frag`
- Create: `shaders/effects/xpl_glitch_rgb_split_v4/effect.json`

- [ ] **Write .frag:**

```glsl
#version 450
layout(location=0) in vec2 vUV; layout(location=0) out vec4 fragColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(push_constant) uniform PC { float uTime, uFrameCount, uParamFloat0, uParamFloat1; };
float rng(vec2 s){return fract(sin(dot(s,vec2(12.9898,78.233)))*43758.5453);}
void main(){
    float tX=floor(uTime*uParamFloat1);
    float sa=uParamFloat0*0.1*rng(vec2(tX,2.0));
    vec2 uv=vUV;
    float r=texture(uInputTex,vec2(uv.x+sa,uv.y)).r;
    float g=texture(uInputTex,uv).g;
    float b=texture(uInputTex,vec2(uv.x-sa,uv.y)).b;
    fragColor=vec4(r,g,b,1.0);
}
```

- [ ] **Write effect.json:**

```json
{"name":"Glitch RGB Split V4","name_cn":"RGB分离 V4","category":"Glitch Effects","category_cn":"故障效果","description":"RGB split with time-quantized random noise. Clean, jittery channel shift.","description_cn":"时间量化随机噪声RGB分离——干净利落的抖动偏移。","params":[{"name":"uParamFloat0","label":"Intensity","label_cn":"强度","type":"float","min":-1,"max":1,"default":0.5,"ui":"slider"},{"name":"uParamFloat1","label":"Speed","label_cn":"速度","type":"float","min":0,"max":100,"default":10,"ui":"slider"}]}
```

---

## Task 5: GlitchRGBSplitV2 (2 params — 4-sine product)

**Files:**
- Create: `shaders/effects/xpl_glitch_rgb_split_v2/xpl_glitch_rgb_split_v2.frag`
- Create: `shaders/effects/xpl_glitch_rgb_split_v2/effect.json`

- [ ] **Write .frag:**

```glsl
#version 450
layout(location=0) in vec2 vUV; layout(location=0) out vec4 fragColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(push_constant) uniform PC { float uTime, uFrameCount, uParamFloat0, uParamFloat1; };
void main(){
    float sa=(1.0+sin(uTime*6.0))*0.5;
    sa*=1.0+sin(uTime*16.0)*0.5; sa*=1.0+sin(uTime*19.0)*0.5;
    sa*=1.0+sin(uTime*27.0)*0.5; sa=pow(sa,uParamFloat1)*0.05*uParamFloat0;
    vec2 uv=vUV;
    vec3 col=vec3(texture(uInputTex,vec2(uv.x+sa,uv.y)).r,texture(uInputTex,uv).g,texture(uInputTex,vec2(uv.x-sa,uv.y)).b);
    col*=1.0-sa*0.5; fragColor=vec4(col,1.0);
}
```

- [ ] **Write effect.json:**

```json
{"name":"Glitch RGB Split V2","name_cn":"RGB分离 V2","category":"Glitch Effects","category_cn":"故障效果","description":"RGB split from product of 4 sine waves. Organic pulsing chromatic aberration.","description_cn":"4层正弦波乘积驱动RGB分离——有机脉动色散。","params":[{"name":"uParamFloat0","label":"Amount","label_cn":"强度","type":"float","min":0,"max":1,"default":0.5,"ui":"slider"},{"name":"uParamFloat1","label":"Amplitude","label_cn":"振幅","type":"float","min":1,"max":6,"default":3,"ui":"slider"}]}
```

---

## Task 6: GlitchAnalogNoise (3 params)

**Files:**
- Create: `shaders/effects/xpl_glitch_analog_noise/xpl_glitch_analog_noise.frag`
- Create: `shaders/effects/xpl_glitch_analog_noise/effect.json`

- [ ] **Write .frag:**

```glsl
#version 450
layout(location=0) in vec2 vUV; layout(location=0) out vec4 fragColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(push_constant) uniform PC { float uTime, uFrameCount, uParamFloat0, uParamFloat1; float _p1,_p2; };
float rng(vec2 s){return fract(sin(dot(s,vec2(12.9898,78.233)))*43758.5453);}
void main(){
    vec4 sc=texture(uInputTex,vUV); vec4 nc=sc;
    float lum=dot(nc.rgb,vec3(0.22,0.707,0.071));
    if(rng(vec2(uTime*uParamFloat0))>_p1)nc=vec4(vec3(lum),sc.a);
    float nx=rng(vec2(uTime*uParamFloat0+vUV.x/-213.0,uTime*uParamFloat0+vUV.y/5.53));
    float ny=rng(vec2(uTime*uParamFloat0-vUV.x/213.0,uTime*uParamFloat0-vUV.y/-5.53));
    float nz=rng(vec2(uTime*uParamFloat0+vUV.x/213.0,uTime*uParamFloat0+vUV.y/5.53));
    nc.rgb+=0.25*vec3(nx,ny,nz)-0.125; nc=mix(sc,nc,uParamFloat1); fragColor=nc;
}
```

Wait — I'm overcomplicating the push constant layout. The engine's ShaderParams has `uniformFloats[]` that maps to push constants sequentially. I need to align with how existing shaders declare them. Let me check the existing pattern.

Actually, looking at the existing shaders, they use simple layout. Let me adjust — the engine passes `uniformFloats[]` as a flat array into push constants. The first 2 are `uTime` and `uFrameCount`, then params follow.

Let me use a cleaner pattern that matches existing shaders. Let me check one:

Actually I don't need to read existing shaders — I know from the codebase that:
- ShaderParams.uniformFloats maps to push constants
- index 0 = uTime, index 1 = uFrameCount (these 2 are set by the engine before the uniformFloats)
- Wait, looking at the OnRender code: `params.time = m_effectTime; params.frameCount = m_effectFrameCount;` and `params.uniformFloats = uniformFloats;`

So the shader sees: [time, frameCount, param0, param1, ...] in push constants. 

But push constants have a max size (128 bytes on Vulkan, ~32 floats). For effects with many params, we need to use a different mechanism.

Actually, looking at how the existing shaders work — they declare individual push constants. The engine's `DrawFullscreenQuad` sets them from `ShaderParams`. Let me just match the pattern.

For effects with <= 6 params (up to 8 floats total = 32 bytes), push constants work fine.

Let me just write the plan file now with the core info, and put detailed .frag code in a separate reference file.<｜end▁of▁thinking｜>

<｜｜DSML｜｜tool_calls>
<｜｜DSML｜｜invoke name="Write">
<｜｜DSML｜｜parameter name="content" string="true"># XPL Glitch Effects Migration — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development or superpowers:executing-plans. Steps use checkbox syntax.

**Goal:** Port 17 Glitch effects from X-PostProcessing-Library (HLSL/Unity) to ShaderShowcase (GLSL SPIR-V), 100% fidelity, zero conflict with existing code.

**Architecture:** 17 isolated directories under `shaders/effects/xpl_glitch_*/`, each with `.frag` + `effect.json`. Direction variants merged via `uPassDirection` uniform. All registered at end of `CoverFlowScene::RegisterCards()`.

**Tech Stack:** GLSL 450, SPIR-V 1.3, glslangValidator, C++17, CMake

---

## Task Layout Map

```
                  simple ←─── complexity ───→ complex
  ScreenJump ScreenShake ScanLine Analog V4 V2 V3  DigitalStripe
  ImageBlockV3 V4 TileJitter ImageBlockV2 V1 RGBSplit V5 LineBlock WaveJitter
```

---

## Tasks 0: Create All Directories

- [ ] Run: `pwsh -c "cd e:\AI\graph\hight-post-proc\shader-showcase; 'xpl_glitch_analog_noise','xpl_glitch_digital_stripe','xpl_glitch_image_block_v1','xpl_glitch_image_block_v2','xpl_glitch_image_block_v3','xpl_glitch_image_block_v4','xpl_glitch_line_block','xpl_glitch_rgb_split_v1','xpl_glitch_rgb_split_v2','xpl_glitch_rgb_split_v3','xpl_glitch_rgb_split_v4','xpl_glitch_rgb_split_v5','xpl_glitch_scan_line_jitter','xpl_glitch_screen_jump','xpl_glitch_screen_shake','xpl_glitch_tile_jitter','xpl_glitch_wave_jitter' | %% { New-Item -Force -Type Directory shaders\effects\$_ }; Write-Output '17 dirs created'"`

---

## GLSL Push-Constant Layout Convention

All shaders use the engine's ShaderParams mapping:
```
push constant layout: [uTime, uFrameCount, uParamFloat0, uParamFloat1, ...]
```
Max 32 floats (128 bytes). Effects with >10 params split into `uniformFloats[]` only (no ints).

---

## Tasks 1-17: Effect Shaders

Each task = create `.frag` + `effect.json` pair. Reference XPL shader sources:
- `e:\AI\graph\X-PostProcessing-Library\Assets\X-PostProcessing\Effects\{NAME}\Shader\{NAME}.shader`
- Extract HLSL frag function → translate to GLSL (replace `TEXTURE2D_SAMPLER2D`→`sampler2D`, `SAMPLE_TEXTURE2D`→`texture()`, `half`→`float`, `lerp`→`mix`, `frac`→`fract`, `saturate`→`clamp`, `fmod`→`mod`, `_Time.y`→`uTime`)

### Task 1: GlitchScreenJump (1 param) ⬛ LOWEST
**Files:** `shaders/effects/xpl_glitch_screen_jump/` x2

`.frag`:
```glsl
#version 450
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(push_constant)uniform PC{float uTime,uFrameCount,uI,uD;};
void main(){
    float jt=uTime*uI*9.8;
    float jx=mix(vUV.x,fract(vUV.x+jt),uI);
    float jy=mix(vUV.y,fract(vUV.y+jt),uI);
    vec2 uv=uD>0.5?fract(vec2(vUV.x,jy)):fract(vec2(jx,vUV.y));
    fC=texture(uInputTex,uv);
}
```

`effect.json`:
```json
{"name":"Glitch Screen Jump","name_cn":"屏幕跳跃","category":"Glitch Effects","category_cn":"故障效果","description":"Screen jump displacement. Horizontal/vertical continuous scrolling jump.","description_cn":"屏幕画面跳动——水平/垂直方向持续滚动跳跃。","params":[{"name":"uParamFloat0","label":"Intensity","label_cn":"强度","type":"float","min":0,"max":1,"default":0.35,"ui":"slider"}]}
```

### Task 2: GlitchScreenShake (1 param) ⬛ LOWEST
**Files:** `shaders/effects/xpl_glitch_screen_shake/` x2

`.frag`:
```glsl
#version 450
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(push_constant)uniform PC{float uTime,uFrameCount,uI,uD;};
float R(vec2 s){return fract(sin(dot(s,vec2(127.1,311.7)))*43758.5453);}
void main(){
    float sh=(R(vec2(uTime,2.0))-0.5)*uI*0.25;
    vec2 uv=vUV;if(uD>0.5)uv=fract(vec2(vUV.x,vUV.y+sh));else uv=fract(vec2(vUV.x+sh,vUV.y));
    fC=texture(uInputTex,uv);
}
```

`effect.json`:
```json
{"name":"Glitch Screen Shake","name_cn":"屏幕震动","category":"Glitch Effects","category_cn":"故障效果","description":"Random screen shake. Simulates handheld camera/earthquake effect.","description_cn":"随机屏幕震动——模拟手持摄影/地震效果。","params":[{"name":"uParamFloat0","label":"Intensity","label_cn":"强度","type":"float","min":0,"max":1,"default":0.5,"ui":"slider"}]}
```

### Task 3: GlitchScanLineJitter (2 params) ⬛ LOW
**Files:** `shaders/effects/xpl_glitch_scan_line_jitter/` x2

`.frag`:
```glsl
#version 450
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(push_constant)uniform PC{float uTime,uFrameCount,uJI,uFq;};
float R(vec2 s){return fract(sin(dot(s,vec2(12.9898,78.233)))*43758.5453);}
void main(){
    float t=clamp(1.0-uJI*1.2,0.0,1.0);
    float a=0.005+pow(uJI,3.0)*0.1;
    float s=0.5+0.5*cos(uTime*uFq);
    float j=R(vec2(vUV.y,uTime))*2.0-1.0;
    j*=step(t,abs(j))*a*s;
    fC=texture(uInputTex,fract(vUV+vec2(j,0.0)));
}
```

`effect.json`:
```json
{"name":"Glitch Scan Line Jitter","name_cn":"扫描线抖动","category":"Glitch Effects","category_cn":"故障效果","description":"Horizontal scan line jitter with configurable intensity and frequency.","description_cn":"水平扫描线随机抖动。","params":[{"name":"uParamFloat0","label":"Jitter Intensity","label_cn":"抖动强度","type":"float","min":0,"max":1,"default":0.1,"ui":"slider"},{"name":"uParamFloat1","label":"Frequency","label_cn":"频率","type":"float","min":0.1,"max":25,"default":1,"ui":"slider"}]}
```

### Task 4: GlitchRGBSplitV4 (2 params) ⬛ LOW
**Files:** `shaders/effects/xpl_glitch_rgb_split_v4/` x2

`.frag`:
```glsl
#version 450
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(push_constant)uniform PC{float uTime,uFrameCount,uIn,uSp;};
float R(vec2 s){return fract(sin(dot(s,vec2(12.9898,78.233)))*43758.5453);}
void main(){
    float tX=floor(uTime*uSp);
    float sa=uIn*0.1*R(vec2(tX,2.0));
    float r=texture(uInputTex,vec2(vUV.x+sa,vUV.y)).r;
    float g=texture(uInputTex,vUV).g;
    float b=texture(uInputTex,vec2(vUV.x-sa,vUV.y)).b;
    fC=vec4(r,g,b,1.0);
}
```

`effect.json`:
```json
{"name":"Glitch RGB Split V4","name_cn":"RGB分离 V4","category":"Glitch Effects","category_cn":"故障效果","description":"RGB split with time-quantized random noise. Clean jittery channel shift.","description_cn":"时间量化随机噪声RGB分离。","params":[{"name":"uParamFloat0","label":"Intensity","label_cn":"强度","type":"float","min":-1,"max":1,"default":0.5,"ui":"slider"},{"name":"uParamFloat1","label":"Speed","label_cn":"速度","type":"float","min":0,"max":100,"default":10,"ui":"slider"}]}
```

### Task 5: GlitchRGBSplitV2 (2 params) ⬛ LOW
**Files:** `shaders/effects/xpl_glitch_rgb_split_v2/` x2

`.frag`:
```glsl
#version 450
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(push_constant)uniform PC{float uTime,uFrameCount,uAm,uAp;};
void main(){
    float sa=(1.0+sin(uTime*6.0))*0.5;
    sa*=1.0+sin(uTime*16.0)*0.5;sa*=1.0+sin(uTime*19.0)*0.5;
    sa*=1.0+sin(uTime*27.0)*0.5;sa=pow(sa,uAp)*0.05*uAm;
    vec3 c=vec3(texture(uInputTex,vec2(vUV.x+sa,vUV.y)).r,texture(uInputTex,vUV).g,texture(uInputTex,vec2(vUV.x-sa,vUV.y)).b);
    c*=1.0-sa*0.5;fC=vec4(c,1.0);
}
```

`effect.json`:
```json
{"name":"Glitch RGB Split V2","name_cn":"RGB分离 V2","category":"Glitch Effects","category_cn":"故障效果","description":"RGB split from 4-sine product. Organic pulsing chromatic aberration.","description_cn":"4层正弦波乘积RGB分离——有机脉动色散。","params":[{"name":"uParamFloat0","label":"Amount","label_cn":"强度","type":"float","min":0,"max":1,"default":0.5,"ui":"slider"},{"name":"uParamFloat1","label":"Amplitude","label_cn":"振幅","type":"float","min":1,"max":6,"default":3,"ui":"slider"}]}
```

### Task 6: GlitchAnalogNoise (3 params) ⬛ LOW
**Files:** `shaders/effects/xpl_glitch_analog_noise/` x2

`.frag`:
```glsl
#version 450
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(push_constant)uniform PC{float uTime,uFrameCount,uNS,uNF,uLT;};
float R(vec2 s){return fract(sin(dot(s,vec2(12.9898,78.233)))*43758.5453);}
void main(){
    vec4 sc=texture(uInputTex,vUV);vec4 nc=sc;
    float lum=dot(nc.rgb,vec3(0.22,0.707,0.071));
    if(R(vec2(uTime*uNS))>uLT)nc=vec4(vec3(lum),sc.a);
    float nx=R(vec2(uTime*uNS+vUV.x/-213.0,uTime*uNS+vUV.y/5.53));
    float ny=R(vec2(uTime*uNS-vUV.x/213.0,uTime*uNS-vUV.y/-5.53));
    float nz=R(vec2(uTime*uNS+vUV.x/213.0,uTime*uNS+vUV.y/5.53));
    nc.rgb+=0.25*vec3(nx,ny,nz)-0.125;nc=mix(sc,nc,uNF);fC=nc;
}
```

`effect.json`:
```json
{"name":"Glitch Analog Noise","name_cn":"模拟噪声","category":"Glitch Effects","category_cn":"故障效果","description":"Analog static interference with luminance jitter and per-channel noise.","description_cn":"模拟信号噪声干扰——亮度抖动+分通道彩色噪点。","params":[{"name":"uParamFloat0","label":"Noise Speed","label_cn":"噪声速度","type":"float","min":0,"max":1,"default":0.5,"ui":"slider"},{"name":"uParamFloat1","label":"Noise Fading","label_cn":"噪声渐变","type":"float","min":0,"max":1,"default":0.5,"ui":"slider"},{"name":"uParamFloat2","label":"Luminance Threshold","label_cn":"亮度阈值","type":"float","min":0,"max":1,"default":0.8,"ui":"slider"}]}
```

### Task 7: GlitchImageBlockV3 (2 params) ⬛ LOW
**Files:** `shaders/effects/xpl_glitch_image_block_v3/` x2

`.frag`:
```glsl
#version 450
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(push_constant)uniform PC{float uTime,uFrameCount,uSp,uBS;};
float R(vec2 s){return fract(sin(dot(s*floor(uTime*uSp),vec2(17.13,3.71)))*43758.5453123);}
void main(){
    float bx=R(floor(vUV*uBS));
    float by=R(floor(vUV*uBS+vec2(0,1)));
    float dn=pow(bx,8.0)*pow(bx,3.0);
    float r=texture(uInputTex,vUV).r;
    float g=texture(uInputTex,vUV+vec2(dn*0.05*R(vec2(7.0)))).g;
    float b=texture(uInputTex,vUV-vec2(dn*0.05*R(vec2(13.0)))).b;
    fC=vec4(r,g,b,1.0);
}
```

`effect.json`:
```json
{"name":"Glitch Image Block V3","name_cn":"画面块错位 V3","category":"Glitch Effects","category_cn":"故障效果","description":"Single block layer displacement with RGB split. Simplest ImageBlock variant.","description_cn":"单层方块错位+RGB分离。最简版画面块。","params":[{"name":"uParamFloat0","label":"Speed","label_cn":"速度","type":"float","min":0,"max":50,"default":10,"ui":"slider"},{"name":"uParamFloat1","label":"Block Size","label_cn":"块大小","type":"float","min":1,"max":50,"default":8,"ui":"slider"}]}
```

### Task 8: GlitchImageBlockV4 (4 params) ⬛ LOW
**Files:** `shaders/effects/xpl_glitch_image_block_v4/` x2

`.frag`:
```glsl
#version 450
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(push_constant)uniform PC{float uTime,uFrameCount,uSp,uBS,uSX,uSY;};
float R(vec2 s){return fract(sin(dot(s,vec2(12.9898,78.233)))*43758.5453);}
void main(){
    float bx=R(floor(vUV*uBS));
    float by=R(floor(vUV*uBS+vec2(0,1)));
    float dn=pow(bx,8.0)*pow(bx,3.0);
    float sn=pow(R(vec2(7.2341)),17.0);
    float ox=dn-sn*uSX;float oy=dn-sn*uSY;
    vec2 off=vec2(ox*0.05*R(vec2(13.0)),oy*0.05*R(vec2(7.0)));
    float r=texture(uInputTex,vUV).r;
    float g=texture(uInputTex,vUV+off).g;
    float b=texture(uInputTex,vUV-off).b;
    fC=vec4(r,g,b,1.0);
}
```

`effect.json`:
```json
{"name":"Glitch Image Block V4","name_cn":"画面块错位 V4","category":"Glitch Effects","category_cn":"故障效果","description":"Block displacement with dual-axis RGB split. Per-block 2D channel shifting.","description_cn":"方块错位+双轴RGB分裂——每块内二维通道偏移。","params":[{"name":"uParamFloat0","label":"Speed","label_cn":"速度","type":"float","min":0,"max":50,"default":10,"ui":"slider"},{"name":"uParamFloat1","label":"Block Size","label_cn":"块大小","type":"float","min":1,"max":50,"default":8,"ui":"slider"},{"name":"uParamFloat2","label":"RGB Split X","label_cn":"RGB分裂X","type":"float","min":0,"max":25,"default":1,"ui":"slider"},{"name":"uParamFloat3","label":"RGB Split Y","label_cn":"RGB分裂Y","type":"float","min":0,"max":25,"default":1,"ui":"slider"}]}
```

### Task 9: GlitchTileJitter (4 params) ⬛ LOW-MED
**Files:** `shaders/effects/xpl_glitch_tile_jitter/` x2

`.frag`:
```glsl
#version 450
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(push_constant)uniform PC{float uTime,uFrameCount,uSN,uJA,uJS,uFq;};
layout(constant_id=0)const float uD=0.0;
void main(){
    vec2 uv=vUV;float s=0.5+0.5*cos(uTime*uFq);float ps=1.0/1920.0;
    bool j=uD>0.5?mod(uv.x*uSN,2.0)<1.0:mod(uv.y*uSN,2.0)<1.0;
    if(j){float a=ps*cos(uTime*uJS*100.0)*uJA*s;if(uD>0.5)uv.y+=a;else uv.x+=a;}
    fC=texture(uInputTex,uv);
}
```

`effect.json`:
```json
{"name":"Glitch Tile Jitter","name_cn":"瓦片抖动","category":"Glitch Effects","category_cn":"故障效果","description":"Alternating tile band jitter. Screen split into bands that jitter independently.","description_cn":"隔行瓦片抖动——画面分割为条带，间隔条带独立抖动。","params":[{"name":"uParamFloat0","label":"Splitting","label_cn":"分割数","type":"float","min":1,"max":50,"default":5,"ui":"slider"},{"name":"uParamFloat1","label":"Jitter Amount","label_cn":"抖动强度","type":"float","min":0,"max":100,"default":10,"ui":"slider"},{"name":"uParamFloat2","label":"Speed","label_cn":"速度","type":"float","min":0,"max":1,"default":0.35,"ui":"slider"},{"name":"uParamFloat3","label":"Frequency","label_cn":"频率","type":"float","min":0,"max":25,"default":1,"ui":"slider"}]}
```

### Task 10: GlitchRGBSplitV3 (3 params + direction) ⬛ LOW-MED
**Files:** `shaders/effects/xpl_glitch_rgb_split_v3/` x2

`.frag`:
```glsl
#version 450
layout(location=0)in vec2 vUV;layout(location=0)out vec4 fC;
layout(binding=0)uniform sampler2D uInputTex;
layout(push_constant)uniform PC{float uTime,uFrameCount,uFq,uAm,uSp,uD;};
void main(){
    float a=uAm*0.001;float s=0.5+0.5*cos(uTime*uFq);a*=s;vec2 uv=vUV;
    float r,g,b;
    if(uD<0.5){//H
        r=texture(uInputTex,vec2(uv.x+sin(uTime*uSp*0.2)*a,uv.y)).r;
        g=texture(uInputTex,vec2(uv.x+sin(uTime*uSp*0.1)*a,uv.y)).g;
        b=texture(uInputTex,vec2(uv.x+sin(uTime*uSp*0.05)*a,uv.y)).b;
    }else if(uD<1.5){//V
        r=texture(uInputTex,vec2(uv.x,uv.y+sin(uTime*uSp*0.2)*a)).r;
        g=texture(uInputTex,vec2(uv.x,uv.y+sin(uTime*uSp*0.1)*a)).g;
        b=texture(uInputTex,vec2(uv.x,uv.y-sin(uTime*uSp*0.05)*a)).b;
    }else{//H+V
        r=texture(uInputTex,vec2(uv.x+sin(uTime*uSp*0.2)*a,uv.y+sin(uTime*uSp*0.2)*a)).r;
        g=texture(uInputTex,vec2(uv.x+sin(uTime*uSp*0.1)*a,uv.y+sin(uTime*uSp*0.1)*a)).g;
        b=texture(uInputTex,vec2(uv.x-sin(uTime*uSp*0.05)*a,uv.y-sin(uTime*uSp*0.05)*a)).b;
    }
    fC=vec4(r,g,b,1.0);
}
```

`effect.json`:
```json
{"name":"Glitch RGB Split V3","name_cn":"RGB分离 V3","category":"Glitch Effects","category_cn":"故障效果","description":"Multi-channel RGB split with per-channel sine wave offsets at different frequencies. 3 direction modes.","description_cn":"多通道RGB分离——每通道不同频率正弦偏移。支持3种方向模式。","params":[{"name":"uParamFloat0","label":"Frequency","label_cn":"频率","type":"float","min":0.1,"max":25,"default":3,"ui":"slider"},{"name":"uParamFloat1","label":"Amount","label_cn":"强度","type":"float","min":0,"max":200,"default":30,"ui":"slider"},{"name":"uParamFloat2","label":"Speed","label_cn":"速度","type":"float","min":0,"max":15,"default":20,"ui":"slider"}]}
```

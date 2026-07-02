#include "render/DissolvePostProcess.h"
#include <cstdio>

bool DissolvePostProcess::Init(IRenderBackend* backend) { return false; }
void DissolvePostProcess::Apply(IRenderBackend* backend, TextureHandle inputRT, TextureHandle outputRT,
                                 float dissolveAmount, float edgeWidth,
                                 float noiseScaleX, float noiseScaleY,
                                 float noiseOffsetX, float noiseOffsetY) {}
void DissolvePostProcess::Destroy(IRenderBackend* backend) {}
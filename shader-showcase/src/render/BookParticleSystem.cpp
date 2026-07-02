#include "render/BookParticleSystem.h"
#include <cmath>

void BookParticleSystem::Init(const ParticleConfig& config) { m_config = config; m_particles.resize(config.capacity); m_rng.seed(42); }
void BookParticleSystem::Update(float dt, float dissolveAmount) {}
void BookParticleSystem::Render(IRenderBackend* backend, ShaderHandle vert, ShaderHandle frag,
                                 const float* viewMat, const float* projMat) {}
void BookParticleSystem::Destroy(IRenderBackend* backend) { m_particles.clear(); }
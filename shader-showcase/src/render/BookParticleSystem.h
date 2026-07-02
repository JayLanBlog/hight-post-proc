#pragma once
#include "render/IRenderBackend.h"
#include <vector>
#include <random>

struct Particle {
    float posX, posY, posZ;
    float velX, velY, velZ;
    float life;
    float maxLife;
    float size;
    float rotation;
    float angularVel;
    bool alive = false;
};

struct ParticleConfig {
    uint32_t capacity = 32;
    float spawnRate = 16.0f;
    float spawnCenter[3] = {0, 0, 0};
    float spawnSize[3] = {3, 4, 3};
    float velBase[3] = {0, 0, 0};
    float velRange[3] = {0, 0, 0};
    float lifeMin = 1.0f;
    float lifeMax = 3.0f;
    float sizeMin = 0.5f;
    float sizeMax = 1.0f;
    float angularVelMin = 0.0f;
    float angularVelMax = 0.0f;
    float gravity[3] = {0, 0, 0};
};

class BookParticleSystem {
public:
    void Init(const ParticleConfig& config);
    void Update(float dt, float dissolveAmount);
    void Render(IRenderBackend* backend, ShaderHandle vert, ShaderHandle frag,
               const float* viewMat, const float* projMat);
    void Destroy(IRenderBackend* backend);
    int GetAliveCount() const { return m_aliveCount; }
    void SetParticleTexture(TextureHandle tex) { m_particleTex = tex; }

private:
    ParticleConfig m_config;
    std::vector<Particle> m_particles;
    std::mt19937 m_rng;
    float m_spawnAccum = 0.0f;
    TextureHandle m_particleTex = {0};
    uint32_t m_aliveCount = 0;
};
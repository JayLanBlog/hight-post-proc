#include "render/BookParticleSystem.h"
#include <cmath>
#include <cstdio>

void BookParticleSystem::Init(const ParticleConfig& config) {
    m_config = config;
    m_particles.resize(config.capacity);
    m_rng.seed(42);
    m_spawnAccum = 0.0f;
    m_aliveCount = 0;
}

void BookParticleSystem::Update(float dt, float dissolveAmount) {
    // 更新存活粒子
    for (auto& p : m_particles) {
        if (!p.alive) continue;
        p.life -= dt;
        if (p.life <= 0.0f) {
            p.alive = false;
            continue;
        }
        // 应用重力
        p.velX += m_config.gravity[0] * dt;
        p.velY += m_config.gravity[1] * dt;
        p.velZ += m_config.gravity[2] * dt;
        // 更新位置
        p.posX += p.velX * dt;
        p.posY += p.velY * dt;
        p.posZ += p.velZ * dt;
        // 旋转
        p.rotation += p.angularVel * dt;
    }

    // 产生新粒子
    m_spawnAccum += m_config.spawnRate * dt;
    int toSpawn = (int)m_spawnAccum;
    m_spawnAccum -= (float)toSpawn;

    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
    for (int i = 0; i < toSpawn; i++) {
        // 找空闲粒子槽
        int slot = -1;
        for (int j = 0; j < (int)m_config.capacity; j++) {
            if (!m_particles[j].alive) { slot = j; break; }
        }
        if (slot < 0) break; // 所有粒子都活着

        auto& p = m_particles[slot];
        p.alive = true;
        p.posX = m_config.spawnCenter[0] + (dist01(m_rng) - 0.5f) * m_config.spawnSize[0];
        p.posY = m_config.spawnCenter[1] + (dist01(m_rng) - 0.5f) * m_config.spawnSize[1];
        p.posZ = m_config.spawnCenter[2] + (dist01(m_rng) - 0.5f) * m_config.spawnSize[2];
        p.velX = m_config.velBase[0] + (dist01(m_rng) - 0.5f) * m_config.velRange[0];
        p.velY = m_config.velBase[1] + (dist01(m_rng) - 0.5f) * m_config.velRange[1];
        p.velZ = m_config.velBase[2] + (dist01(m_rng) - 0.5f) * m_config.velRange[2];
        p.maxLife = m_config.lifeMin + dist01(m_rng) * (m_config.lifeMax - m_config.lifeMin);
        p.life = p.maxLife;
        p.size = m_config.sizeMin + dist01(m_rng) * (m_config.sizeMax - m_config.sizeMin);
        p.angularVel = m_config.angularVelMin + dist01(m_rng) * (m_config.angularVelMax - m_config.angularVelMin);
        p.rotation = dist01(m_rng) * 6.28318f;
    }

    // 统计存活数
    m_aliveCount = 0;
    for (auto& p : m_particles) if (p.alive) m_aliveCount++;
}

void BookParticleSystem::Render(IRenderBackend* backend, ShaderHandle vert, ShaderHandle frag,
                                 const float* viewMat, const float* projMat) {
    if (m_aliveCount == 0) return;

    // 构建顶点数据: 每粒子8个float (32字节)
    // layout: pos(3) + size/life/rotation(3) + padding(2)
    std::vector<float> vertData;
    vertData.reserve(m_aliveCount * 8);
    for (auto& p : m_particles) {
        if (!p.alive) continue;
        vertData.push_back(p.posX);
        vertData.push_back(p.posY);
        vertData.push_back(p.posZ);
        vertData.push_back(p.size);
        vertData.push_back(p.life / p.maxLife);
        vertData.push_back(p.rotation);
        vertData.push_back(0.0f);  // padding
        vertData.push_back(0.0f);  // padding
    }

    // 计算MVP矩阵
    float mvp[16];
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            float sum = 0;
            for (int k = 0; k < 4; k++)
                sum += projMat[r*4 + k] * viewMat[k*4 + c];
            mvp[r*4 + c] = sum;
        }
    }

    ShaderParams params;
    if (m_particleTex.id != 0) {
        params.inputTextures.push_back(m_particleTex);
    }
    params.mvp = std::vector<float>(mvp, mvp + 16);
    params.modelView = std::vector<float>(viewMat, viewMat + 16);
    params.uniformFloats = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    params.blendEnable = true;
    params.topology = PrimitiveTopology::PointList;

    backend->DrawMesh(vert, frag, params,
                      vertData.data(), m_aliveCount, 8 * sizeof(float),
                      nullptr, 0);
}

void BookParticleSystem::Destroy(IRenderBackend* backend) {
    if (m_particleTex.id) {
        backend->DestroyTexture(m_particleTex);
        m_particleTex = {0};
    }
    m_particles.clear();
    m_aliveCount = 0;
}
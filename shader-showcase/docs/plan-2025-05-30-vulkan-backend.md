# Vulkan 后端完整实现计划

> **For agentic workers# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 img# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。
# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h`# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 img# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---
# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` -# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRender# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    Vk# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height =# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    Vk# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    //# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown()# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // Im# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGui# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height)# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, Texture# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle,# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    //# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRender# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreen# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag,# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void Set# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    Vk# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue =# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_present# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily =# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swap# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    Vk# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;

    // 同步对象
    VkSemaphore m_imgAvailSem = VK_NULL_HANDLE# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;

    // 同步对象
    VkSemaphore m_imgAvailSem = VK_NULL_HANDLE;
    VkSemaphore m_renderDoneSem =# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;

    // 同步对象
    VkSemaphore m_imgAvailSem = VK_NULL_HANDLE;
    VkSemaphore m_renderDoneSem = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;

    // Im# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;

    // 同步对象
    VkSemaphore m_imgAvailSem = VK_NULL_HANDLE;
    VkSemaphore m_renderDoneSem = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;

    // ImGui
    VkDescriptorPool m_imguiDesc# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;

    // 同步对象
    VkSemaphore m_imgAvailSem = VK_NULL_HANDLE;
    VkSemaphore m_renderDoneSem = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;

    // ImGui
    VkDescriptorPool m_imguiDescPool = VK_NULL_HANDLE;
    VkRender# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;

    // 同步对象
    VkSemaphore m_imgAvailSem = VK_NULL_HANDLE;
    VkSemaphore m_renderDoneSem = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;

    // ImGui
    VkDescriptorPool m_imguiDescPool = VK_NULL_HANDLE;
    VkRenderPass m_imguiRenderPass = VK_NULL_HANDLE# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;

    // 同步对象
    VkSemaphore m_imgAvailSem = VK_NULL_HANDLE;
    VkSemaphore m_renderDoneSem = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;

    // ImGui
    VkDescriptorPool m_imguiDescPool = VK_NULL_HANDLE;
    VkRenderPass m_imguiRenderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_imguiFramebuffers;

    //# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;

    // 同步对象
    VkSemaphore m_imgAvailSem = VK_NULL_HANDLE;
    VkSemaphore m_renderDoneSem = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;

    // ImGui
    VkDescriptorPool m_imguiDescPool = VK_NULL_HANDLE;
    VkRenderPass m_imguiRenderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_imguiFramebuffers;

    // 当前帧状态
    uint32_t m_currentImageIndex = 0;
    bool# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;

    // 同步对象
    VkSemaphore m_imgAvailSem = VK_NULL_HANDLE;
    VkSemaphore m_renderDoneSem = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;

    // ImGui
    VkDescriptorPool m_imguiDescPool = VK_NULL_HANDLE;
    VkRenderPass m_imguiRenderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_imguiFramebuffers;

    // 当前帧状态
    uint32_t m_currentImageIndex = 0;
    bool m_frameStarted = false;
    bool m_renderToTexture = false;
    VulkanTexture# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;

    // 同步对象
    VkSemaphore m_imgAvailSem = VK_NULL_HANDLE;
    VkSemaphore m_renderDoneSem = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;

    // ImGui
    VkDescriptorPool m_imguiDescPool = VK_NULL_HANDLE;
    VkRenderPass m_imguiRenderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_imguiFramebuffers;

    // 当前帧状态
    uint32_t m_currentImageIndex = 0;
    bool m_frameStarted = false;
    bool m_renderToTexture = false;
    VulkanTexture* m_currentTarget = nullptr;

    // 资源管理
    std::unordered_map# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;

    // 同步对象
    VkSemaphore m_imgAvailSem = VK_NULL_HANDLE;
    VkSemaphore m_renderDoneSem = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;

    // ImGui
    VkDescriptorPool m_imguiDescPool = VK_NULL_HANDLE;
    VkRenderPass m_imguiRenderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_imguiFramebuffers;

    // 当前帧状态
    uint32_t m_currentImageIndex = 0;
    bool m_frameStarted = false;
    bool m_renderToTexture = false;
    VulkanTexture* m_currentTarget = nullptr;

    // 资源管理
    std::unordered_map<uint32_t, std::unique_ptr<VulkanShader>> m_shaders;
    std::# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;

    // 同步对象
    VkSemaphore m_imgAvailSem = VK_NULL_HANDLE;
    VkSemaphore m_renderDoneSem = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;

    // ImGui
    VkDescriptorPool m_imguiDescPool = VK_NULL_HANDLE;
    VkRenderPass m_imguiRenderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_imguiFramebuffers;

    // 当前帧状态
    uint32_t m_currentImageIndex = 0;
    bool m_frameStarted = false;
    bool m_renderToTexture = false;
    VulkanTexture* m_currentTarget = nullptr;

    // 资源管理
    std::unordered_map<uint32_t, std::unique_ptr<VulkanShader>> m_shaders;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanTexture>> m_textures;
# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;

    // 同步对象
    VkSemaphore m_imgAvailSem = VK_NULL_HANDLE;
    VkSemaphore m_renderDoneSem = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;

    // ImGui
    VkDescriptorPool m_imguiDescPool = VK_NULL_HANDLE;
    VkRenderPass m_imguiRenderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_imguiFramebuffers;

    // 当前帧状态
    uint32_t m_currentImageIndex = 0;
    bool m_frameStarted = false;
    bool m_renderToTexture = false;
    VulkanTexture* m_currentTarget = nullptr;

    // 资源管理
    std::unordered_map<uint32_t, std::unique_ptr<VulkanShader>> m_shaders;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanTexture>> m_textures;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanPipeline>> m_pipelines;
    uint32_t m_nextShaderId =# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;

    // 同步对象
    VkSemaphore m_imgAvailSem = VK_NULL_HANDLE;
    VkSemaphore m_renderDoneSem = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;

    // ImGui
    VkDescriptorPool m_imguiDescPool = VK_NULL_HANDLE;
    VkRenderPass m_imguiRenderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_imguiFramebuffers;

    // 当前帧状态
    uint32_t m_currentImageIndex = 0;
    bool m_frameStarted = false;
    bool m_renderToTexture = false;
    VulkanTexture* m_currentTarget = nullptr;

    // 资源管理
    std::unordered_map<uint32_t, std::unique_ptr<VulkanShader>> m_shaders;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanTexture>> m_textures;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanPipeline>> m_pipelines;
    uint32_t m_nextShaderId = 1;
    uint32_t m_next# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;

    // 同步对象
    VkSemaphore m_imgAvailSem = VK_NULL_HANDLE;
    VkSemaphore m_renderDoneSem = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;

    // ImGui
    VkDescriptorPool m_imguiDescPool = VK_NULL_HANDLE;
    VkRenderPass m_imguiRenderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_imguiFramebuffers;

    // 当前帧状态
    uint32_t m_currentImageIndex = 0;
    bool m_frameStarted = false;
    bool m_renderToTexture = false;
    VulkanTexture* m_currentTarget = nullptr;

    // 资源管理
    std::unordered_map<uint32_t, std::unique_ptr<VulkanShader>> m_shaders;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanTexture>> m_textures;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanPipeline>> m_pipelines;
    uint32_t m_nextShaderId = 1;
    uint32_t m_nextTextureId = 1;
    uint32# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;

    // 同步对象
    VkSemaphore m_imgAvailSem = VK_NULL_HANDLE;
    VkSemaphore m_renderDoneSem = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;

    // ImGui
    VkDescriptorPool m_imguiDescPool = VK_NULL_HANDLE;
    VkRenderPass m_imguiRenderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_imguiFramebuffers;

    // 当前帧状态
    uint32_t m_currentImageIndex = 0;
    bool m_frameStarted = false;
    bool m_renderToTexture = false;
    VulkanTexture* m_currentTarget = nullptr;

    // 资源管理
    std::unordered_map<uint32_t, std::unique_ptr<VulkanShader>> m_shaders;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanTexture>> m_textures;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanPipeline>> m_pipelines;
    uint32_t m_nextShaderId = 1;
    uint32_t m_nextTextureId = 1;
    uint32_t m_nextPipelineId = 1;

    // 视口
    int m# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;

    // 同步对象
    VkSemaphore m_imgAvailSem = VK_NULL_HANDLE;
    VkSemaphore m_renderDoneSem = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;

    // ImGui
    VkDescriptorPool m_imguiDescPool = VK_NULL_HANDLE;
    VkRenderPass m_imguiRenderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_imguiFramebuffers;

    // 当前帧状态
    uint32_t m_currentImageIndex = 0;
    bool m_frameStarted = false;
    bool m_renderToTexture = false;
    VulkanTexture* m_currentTarget = nullptr;

    // 资源管理
    std::unordered_map<uint32_t, std::unique_ptr<VulkanShader>> m_shaders;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanTexture>> m_textures;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanPipeline>> m_pipelines;
    uint32_t m_nextShaderId = 1;
    uint32_t m_nextTextureId = 1;
    uint32_t m_nextPipelineId = 1;

    // 视口
    int m_vpX = 0, m_vpY = 0, m_vpWidth = 0, m_vpHeight = 0;
    float# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;

    // 同步对象
    VkSemaphore m_imgAvailSem = VK_NULL_HANDLE;
    VkSemaphore m_renderDoneSem = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;

    // ImGui
    VkDescriptorPool m_imguiDescPool = VK_NULL_HANDLE;
    VkRenderPass m_imguiRenderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_imguiFramebuffers;

    // 当前帧状态
    uint32_t m_currentImageIndex = 0;
    bool m_frameStarted = false;
    bool m_renderToTexture = false;
    VulkanTexture* m_currentTarget = nullptr;

    // 资源管理
    std::unordered_map<uint32_t, std::unique_ptr<VulkanShader>> m_shaders;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanTexture>> m_textures;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanPipeline>> m_pipelines;
    uint32_t m_nextShaderId = 1;
    uint32_t m_nextTextureId = 1;
    uint32_t m_nextPipelineId = 1;

    // 视口
    int m_vpX = 0, m_vpY = 0, m_vpWidth = 0, m_vpHeight = 0;
    float m_clearColor[4] = {0, 0, 0, 1};

    // GLFW 窗口
    GLFW# Vulkan 后端完整实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完整实现 Vulkan 后端，支持所有 18 个 effect 的渲染、ImGui、纹理管理和后端切换

**架构:** 基于 Vulkan 1.3 实现完整的渲染后端，包括实例/设备管理、交换链、命令缓冲区、描述符集、管线状态对象、纹理和帧缓冲区。使用 imgui_impl_vulkan 集成 ImGui。

**技术栈:** Vulkan 1.3, Vulkan SDK 1.3.283, GLFW, ImGui, C++17

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/render/VulkanBackend.h` | Vulkan 后端类声明（大幅扩展） |
| `src/render/VulkanBackend.cpp` | 完整实现（约 1500 行） |
| `src/render/VulkanShader.h` | Vulkan 着色器模块封装 |
| `src/render/VulkanTexture.h/.cpp` | Vulkan 纹理管理 |
| `src/render/VulkanPipeline.h/.cpp` | 管线状态对象管理 |
| `external/imgui/imgui_impl_vulkan.h/.cpp` | ImGui Vulkan 后端（如不存在需添加） |
| `CMakeLists.txt` | 添加 Vulkan 链接和 imgui_impl_vulkan |

---

## Task 1: 重构 VulkanBackend 类结构

**Files:**
- Modify: `src/render/VulkanBackend.h` - 完全重写
- Modify: `src/render/VulkanBackend.cpp` - 清理现有代码

### Step 1.1: 重写 VulkanBackend.h

```cpp
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

struct GLFWwindow;

// Vulkan 资源句柄包装
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 初始化和关闭
    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    // 帧管理
    void BeginFrame() override;
    void EndFrame() override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 视口
    void SetViewport(int x, int y, int width, int height) override;
    void GetViewport(int& x, int& y, int& width, int& height) override;

    // 着色器
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle CreateTextureFBO(int width, int height, TextureFormat format) override;
    void UpdateTexture(TextureHandle handle, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    int GetMaxTextureSize() const override;

    // 渲染
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardRenderData>& cards, ShaderHandle vert, ShaderHandle frag) override;
    void BlitToScreen(TextureHandle src) override;

    // 工具
    void TakeScreenshot(const char* path) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = UINT32_MAX;
    uint32_t m_presentQueueFamily = UINT32_MAX;

    // 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainViews;
    VkFormat m_swapchainFormat;
    VkExtent2D m_swapchainExtent;

    // 命令缓冲区
    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;

    // 同步对象
    VkSemaphore m_imgAvailSem = VK_NULL_HANDLE;
    VkSemaphore m_renderDoneSem = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;

    // ImGui
    VkDescriptorPool m_imguiDescPool = VK_NULL_HANDLE;
    VkRenderPass m_imguiRenderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_imguiFramebuffers;

    // 当前帧状态
    uint32_t m_currentImageIndex = 0;
    bool m_frameStarted = false;
    bool m_renderToTexture = false;
    VulkanTexture* m_currentTarget = nullptr;

    // 资源管理
    std::unordered_map<uint32_t, std::unique_ptr<VulkanShader>> m_shaders;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanTexture>> m_textures;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanPipeline>> m_pipelines;
    uint32_t m_nextShaderId = 1;
    uint32_t m_nextTextureId = 1;
    uint32_t m_nextPipelineId = 1;

    // 视口
    int m_vpX = 0, m_vpY = 0, m_vpWidth = 0, m_vpHeight = 0;
    float m_clearColor[4] = {0, 0, 0, 1};

    // GLFW 窗口
    GLFWwindow* m_window = nullptr;

    // 内部方法
    bool CreateInstance();
    bool SelectPhysicalDevice();
    bool
#include "Shadow.h"
#include <iostream>
#include <GL/glew.h>

// Static method implementation for ShadowManager
glm::mat4 ShadowManager::buildShadowMatrix(const glm::vec3& lightPos, const glm::vec4& plane) {
    float dot = plane.x * lightPos.x + plane.y * lightPos.y + plane.z * lightPos.z + plane.w;
    
    glm::mat4 shadowMat;
    
    shadowMat[0][0] = dot - lightPos.x * plane.x;
    shadowMat[1][0] = 0.0f - lightPos.x * plane.y;
    shadowMat[2][0] = 0.0f - lightPos.x * plane.z;
    shadowMat[3][0] = 0.0f - lightPos.x * plane.w;
    
    shadowMat[0][1] = 0.0f - lightPos.y * plane.x;
    shadowMat[1][1] = dot - lightPos.y * plane.y;
    shadowMat[2][1] = 0.0f - lightPos.y * plane.z;
    shadowMat[3][1] = 0.0f - lightPos.y * plane.w;
    
    shadowMat[0][2] = 0.0f - lightPos.z * plane.x;
    shadowMat[1][2] = 0.0f - lightPos.z * plane.y;
    shadowMat[2][2] = dot - lightPos.z * plane.z;
    shadowMat[3][2] = 0.0f - lightPos.z * plane.w;
    
    shadowMat[0][3] = 0.0f - 1.0f * plane.x;
    shadowMat[1][3] = 0.0f - 1.0f * plane.y;
    shadowMat[2][3] = 0.0f - 1.0f * plane.z;
    shadowMat[3][3] = dot - 1.0f * plane.w;
    
    return shadowMat;
}

// ShadowRenderer implementation
ShadowRenderer::ShadowRenderer() 
    : shadowShader(nullptr), initialized(false), enabled(true),
      groundPlane(0.0f, 1.0f, 0.0f, 0.0f), shadowAlpha(0.5f) {
}

ShadowRenderer::~ShadowRenderer() {
    if (shadowShader) delete shadowShader;
}

bool ShadowRenderer::initialize(const char* shadowVertPath, const char* shadowFragPath) {
    try {
        std::cout << "Initializing shadow renderer..." << std::endl;
        std::cout << "Loading shadow shader: " << shadowVertPath << ", " << shadowFragPath << std::endl;
        shadowShader = new Shader(shadowVertPath, shadowFragPath);
        std::cout << "Shadow shader loaded successfully" << std::endl;
        initialized = true;
        std::cout << "Shadow renderer initialization complete" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize shadow renderer: " << e.what() << std::endl;
        initialized = false;
        return false;
    } catch (...) {
        std::cerr << "Failed to initialize shadow renderer: Unknown error" << std::endl;
        initialized = false;
        return false;
    }
}

glm::mat4 ShadowRenderer::buildShadowProjectionMatrix(const glm::vec3& lightPos, const glm::vec4& plane, bool isDirectional) {
    return ShadowManager::buildShadowMatrix(lightPos, plane);
}

void ShadowRenderer::setGroundPlane(const glm::vec4& plane) {
    groundPlane = plane;
}

void ShadowRenderer::setGroundPlane(float a, float b, float c, float d) {
    groundPlane = glm::vec4(a, b, c, d);
}

void ShadowRenderer::setShadowAlpha(float alpha) {
    shadowAlpha = glm::clamp(alpha, 0.0f, 1.0f);
}

void ShadowRenderer::setEnabled(bool enable) {
    enabled = enable;
}

bool ShadowRenderer::isEnabled() const {
    return enabled && initialized;
}

void ShadowRenderer::renderShadow(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, 
                                 const glm::mat4& projectionMatrix, unsigned int VAO, int vertexCount,
                                 const Light& light) {
    if (!isEnabled()) {
        return;
    }
    
    // 啟用 alpha blending 用於陰影渲染
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 使用 shadow shader
    shadowShader->use();
    
    // 建立陰影投影矩陣
    glm::mat4 shadowMatrix = buildShadowProjectionMatrix(light.getPosition(), groundPlane, 
                                                        light.getType() == LightType::DIRECTIONAL);
    glm::mat4 shadowModel = shadowMatrix * modelMatrix;
    
    // 設定 uniforms
    glUniformMatrix4fv(glGetUniformLocation(shadowShader->ID, "model"), 1, GL_FALSE, glm::value_ptr(shadowModel));
    glUniformMatrix4fv(glGetUniformLocation(shadowShader->ID, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shadowShader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniform1f(glGetUniformLocation(shadowShader->ID, "shadowAlpha"), shadowAlpha);
    
    // 渲染陰影
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    
    // 關閉 alpha blending
    glDisable(GL_BLEND);
}

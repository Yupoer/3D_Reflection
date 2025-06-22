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
    if (isDirectional) {
        // 對於方向光源，使用光源方向而非位置
        // 計算從光源方向到平面的投影矩陣
        glm::vec3 lightDir = glm::normalize(lightPos); // 這裡 lightPos 實際上是方向
        
        // 創建一個遠距離的虛擬光源位置，確保在場景上方
        glm::vec3 virtualLightPos = -lightDir * 100.0f;
        // 確保虛擬光源在地面上方
        if (virtualLightPos.y < 10.0f) {
            virtualLightPos.y = 50.0f; 
        }
        
        
        return ShadowManager::buildShadowMatrix(virtualLightPos, plane);
    } else {
        // 位置光源使用直接投影，確保光源在地面上方
        glm::vec3 adjustedLightPos = lightPos;
        if (adjustedLightPos.y <= 0.1f) {
            adjustedLightPos.y = 1.0f; // 最小高度
        }
        
        
        return ShadowManager::buildShadowMatrix(adjustedLightPos, plane);
    }
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
    
    // 保存當前的 OpenGL 狀態
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    GLint prevBlendSrc, prevBlendDst;
    if (blendEnabled) {
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &prevBlendSrc);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &prevBlendDst);
    }
    
    // 簡化的平面陰影投影 - 使用 alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 禁用深度寫入，避免陰影影響深度緩衝區
    glDepthMask(GL_FALSE);
    
    // 防止 z-fighting，稍微偏移陰影到地面上方
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);
    
    // 使用 shadow shader
    shadowShader->use();
    
    // 建立陰影投影矩陣
    glm::vec3 lightPosOrDir = (light.getType() == LightType::DIRECTIONAL) ? 
                              light.getDirection() : light.getPosition();
    glm::mat4 shadowMatrix = buildShadowProjectionMatrix(lightPosOrDir, groundPlane, 
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
    
    // 恢復 OpenGL 狀態
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDepthMask(GL_TRUE);
    
    // 恢復 blend 狀態
    if (!blendEnabled) {
        glDisable(GL_BLEND);
    } else {
        glBlendFunc(prevBlendSrc, prevBlendDst);
    }
}

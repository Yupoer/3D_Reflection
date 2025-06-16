#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "light.h"

class ShadowRenderer {
private:
    Shader* shadowShader;
    bool initialized;
    bool enabled;
    
    // Ground plane properties
    glm::vec4 groundPlane;  // Ground plane equation (A, B, C, D): Ax + By + Cz + D = 0
    
    // Shadow rendering properties
    float shadowAlpha;
    
    // Helper methods
    glm::mat4 buildShadowProjectionMatrix(const glm::vec3& lightPos, const glm::vec4& plane, bool isDirectional = false);

public:
    ShadowRenderer();
    ~ShadowRenderer();
    
    // Initialization - 只需要 shadow shader
    bool initialize(const char* shadowVertPath, const char* shadowFragPath);
    
    // Configuration
    void setGroundPlane(const glm::vec4& plane);
    void setGroundPlane(float a, float b, float c, float d);
    void setShadowAlpha(float alpha);
    void setEnabled(bool enable);
    bool isEnabled() const;
    
    // 簡化的陰影渲染方法 - 使用 alpha blending
    void renderShadow(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, 
                     const glm::mat4& projectionMatrix, unsigned int VAO, int vertexCount,
                     const Light& light);
};

// Static utility class for shadow matrix calculations
class ShadowManager {
public:
    static glm::mat4 buildShadowMatrix(const glm::vec3& lightPos, const glm::vec4& plane);
};
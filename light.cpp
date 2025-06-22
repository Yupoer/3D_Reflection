#include "Light.h"
#include <iostream>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>  // Comment out if ImGui is not available

// 光源標記（白點）的頂點數據 - 小立方體
static float lightMarkerVertices[] = {
    // 前面
    -0.2f, -0.2f, -0.2f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
     0.2f, -0.2f, -0.2f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,
     0.2f,  0.2f, -0.2f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
     0.2f,  0.2f, -0.2f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
    -0.2f,  0.2f, -0.2f,  0.0f, 1.0f,  0.0f, 0.0f, -1.0f,
    -0.2f, -0.2f, -0.2f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,

    // 後面
    -0.2f, -0.2f,  0.2f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
     0.2f, -0.2f,  0.2f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
     0.2f,  0.2f,  0.2f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
     0.2f,  0.2f,  0.2f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
    -0.2f,  0.2f,  0.2f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
    -0.2f, -0.2f,  0.2f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,

    // 左面
    -0.2f,  0.2f,  0.2f,  1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    -0.2f,  0.2f, -0.2f,  1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -0.2f, -0.2f, -0.2f,  0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -0.2f, -0.2f, -0.2f,  0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -0.2f, -0.2f,  0.2f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    -0.2f,  0.2f,  0.2f,  1.0f, 0.0f, -1.0f, 0.0f, 0.0f,

    // 右面
     0.2f,  0.2f,  0.2f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
     0.2f,  0.2f, -0.2f,  1.0f, 1.0f,  1.0f, 0.0f, 0.0f,
     0.2f, -0.2f, -0.2f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f,
     0.2f, -0.2f, -0.2f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f,
     0.2f, -0.2f,  0.2f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
     0.2f,  0.2f,  0.2f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,

    // 底面
    -0.2f, -0.2f, -0.2f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,
     0.2f, -0.2f, -0.2f,  1.0f, 1.0f,  0.0f, -1.0f, 0.0f,
     0.2f, -0.2f,  0.2f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
     0.2f, -0.2f,  0.2f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
    -0.2f, -0.2f,  0.2f,  0.0f, 0.0f,  0.0f, -1.0f, 0.0f,
    -0.2f, -0.2f, -0.2f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,

    // 頂面
    -0.2f,  0.2f, -0.2f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f,
     0.2f,  0.2f, -0.2f,  1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
     0.2f,  0.2f,  0.2f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
     0.2f,  0.2f,  0.2f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
    -0.2f,  0.2f,  0.2f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
    -0.2f,  0.2f, -0.2f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f
};

// Light 類實現
Light::Light(LightType lightType, glm::vec3 pos_or_dir, glm::vec3 lightColor, float lightIntensity)
    : type(lightType), color(lightColor), intensity(lightIntensity), enabled(true), rotation(0.0f, 0.0f, 0.0f) {
    if (lightType == LightType::POSITIONAL) {
        position = pos_or_dir;
        direction = glm::vec3(0.0f, -1.0f, 0.0f); // 預設方向
    } else {
        direction = glm::normalize(pos_or_dir);
        position = glm::vec3(0.0f); // 方向光源沒有位置
        // 從方向計算初始旋轉角度
        rotation.x = glm::degrees(asin(-direction.y));
        rotation.y = glm::degrees(atan2(direction.x, direction.z));
        rotation.z = 0.0f;
    }
}

void Light::setPosition(const glm::vec3& pos) {
    position = pos;
}

void Light::setDirection(const glm::vec3& dir) {
    direction = glm::normalize(dir);
}

void Light::setRotation(const glm::vec3& rot) {
    rotation = rot;
    updateDirectionFromRotation();
}

void Light::setColor(const glm::vec3& lightColor) {
    color = lightColor;
}

void Light::setIntensity(float lightIntensity) {
    intensity = lightIntensity;
}

void Light::setEnabled(bool isEnabled) {
    enabled = isEnabled;
}

glm::vec3 Light::getPosition() const {
    return position;
}

glm::vec3 Light::getDirection() const {
    return direction;
}

glm::vec3 Light::getRotation() const {
    return rotation;
}

glm::vec3 Light::getColor() const {
    return color;
}

float Light::getIntensity() const {
    return intensity;
}

bool Light::isEnabled() const {
    return enabled;
}

LightType Light::getType() const {
    return type;
}

void Light::updateDirectionFromRotation() {
    if (type == LightType::DIRECTIONAL) {
        // 將歐拉角轉換為方向向量
        glm::mat4 rotationMatrix = glm::mat4(1.0f);
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        
        // 預設方向是向下 (0, -1, 0)
        glm::vec4 defaultDirection = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
        glm::vec4 rotatedDirection = rotationMatrix * defaultDirection;
        
        direction = glm::normalize(glm::vec3(rotatedDirection));
    }
}

void Light::applyToShader(Shader& shader, const std::string& uniformBaseName) const {
    if (!enabled) return;    
    if (type == LightType::POSITIONAL) {
        glUniform3f(glGetUniformLocation(shader.ID, uniformBaseName.c_str()), position.x, position.y, position.z);
        std::string colorUniform = uniformBaseName.substr(0, uniformBaseName.length() - 3) + "Color";
        glm::vec3 finalColor = color * intensity;
        glUniform3f(glGetUniformLocation(shader.ID, colorUniform.c_str()), finalColor.x, finalColor.y, finalColor.z);
    } else {
        glUniform3f(glGetUniformLocation(shader.ID, uniformBaseName.c_str()), direction.x, direction.y, direction.z);
        std::string colorUniform = uniformBaseName.substr(0, uniformBaseName.length() - 3) + "Color";
        glm::vec3 finalColor = color * intensity;
        glUniform3f(glGetUniformLocation(shader.ID, colorUniform.c_str()), finalColor.x, finalColor.y, finalColor.z);
    }
}

void Light::renderImGuiControls(const std::string& lightName, bool isSelected) {
    // 使用不同的顏色顯示選中狀態
    if (isSelected) {
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.7f, 0.2f, 0.8f)); // 綠色表示選中
    }
    
    bool headerExpanded = ImGui::CollapsingHeader(lightName.c_str());
    
    if (isSelected) {
        ImGui::PopStyleColor(); // 在這裡就 Pop，無論 header 是否展開
    }
    
    if (headerExpanded) {
        // 使用 lightName 作為唯一 ID 前綴來避免 ImGui ID 衝突
        ImGui::PushID(lightName.c_str());
        
        ImGui::Checkbox("Enabled", &enabled);
        
        if (type == LightType::POSITIONAL) {
            ImGui::Text("Type: Positional Light");
            // 位置光源只顯示位置控制，不顯示旋轉
            if (ImGui::SliderFloat3("Position", glm::value_ptr(position), 0.0f, 10.0f)) {
                // 位置改變時不需要特殊處理
            }
        } else {
            ImGui::Text("Type: Directional Light");
            // 方向光源只顯示旋轉控制，不顯示位置
            if (ImGui::SliderFloat3("Rotation (degrees)", glm::value_ptr(rotation), -180.0f, 180.0f)) {
                this->updateDirectionFromRotation();
            }
            ImGui::Text("Direction: (%.2f, %.2f, %.2f)", direction.x, direction.y, direction.z);
        }
        
        // 光源顏色固定為白色，不顯示顏色控制
        ImGui::SliderFloat("Intensity", &intensity, 0.0f, 3.0f);
        
        ImGui::PopID(); // 結束唯一 ID 範圍
    }
}

// LightManager 類實現
LightManager::LightManager() : lightMarkerVAO(0), lightMarkerVBO(0) {
    // 添加預設光源，顏色為白色 (1,1,1)
    addPositionalLight(glm::vec3(5.0f, 5.0f, 7.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    addDirectionalLight(glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(1.0f, 1.0f, 1.0f));
    
    initLightMarkerBuffers();
}

LightManager::~LightManager() {
    if (lightMarkerVAO != 0) {
        glDeleteVertexArrays(1, &lightMarkerVAO);
    }
    if (lightMarkerVBO != 0) {
        glDeleteBuffers(1, &lightMarkerVBO);
    }
}

void LightManager::initLightMarkerBuffers() {
    glGenVertexArrays(1, &lightMarkerVAO);
    glGenBuffers(1, &lightMarkerVBO);
    
    glBindVertexArray(lightMarkerVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightMarkerVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lightMarkerVertices), lightMarkerVertices, GL_STATIC_DRAW);
    
    // 位置屬性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 紋理座標屬性
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 法線屬性
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}

void LightManager::addLight(const Light& light) {
    lights.push_back(light);
}

void LightManager::addPositionalLight(const glm::vec3& position, const glm::vec3& color) {
    lights.emplace_back(LightType::POSITIONAL, position, color);
}

void LightManager::addDirectionalLight(const glm::vec3& direction, const glm::vec3& color) {
    lights.emplace_back(LightType::DIRECTIONAL, direction, color);
}

Light& LightManager::getLight(size_t index) {
    return lights[index];
}

const Light& LightManager::getLight(size_t index) const {
    return lights[index];
}

size_t LightManager::getLightCount() const {
    return lights.size();
}

void LightManager::applyAllLightsToShader(Shader& shader) const {
    // 應用第一個光源（位置光源）
    if (lights.size() > 0) {
        const Light& light1 = lights[0];
        glUniform3f(glGetUniformLocation(shader.ID, "lightPos"), 
                   light1.position.x, light1.position.y, light1.position.z);
        glUniform3f(glGetUniformLocation(shader.ID, "lightColor"), 
                   light1.color.x * light1.intensity, 
                   light1.color.y * light1.intensity, 
                   light1.color.z * light1.intensity);
        glUniform1i(glGetUniformLocation(shader.ID, "light1Enabled"), light1.enabled);
    }
    
    // 應用第二個光源（方向光源）
    if (lights.size() > 1) {
        const Light& light2 = lights[1];
        glUniform3f(glGetUniformLocation(shader.ID, "lightPos2"), 
                   light2.direction.x, light2.direction.y, light2.direction.z);
        glUniform3f(glGetUniformLocation(shader.ID, "lightColor2"), 
                   light2.color.x * light2.intensity, 
                   light2.color.y * light2.intensity, 
                   light2.color.z * light2.intensity);
        glUniform1i(glGetUniformLocation(shader.ID, "light2Enabled"), light2.enabled);
    }
}

void LightManager::renderImGuiControls() {
    ImGui::Separator();
    ImGui::Text("Light Controls");
    
    for (size_t i = 0; i < lights.size(); ++i) {
        std::string lightName;        if (lights[i].getType() == LightType::POSITIONAL) {
            lightName = "Positional Light " + std::to_string(i + 1);
        } else {
            lightName = "Directional Light " + std::to_string(i + 1);
        }
        lights[i].renderImGuiControls(lightName, false); // 假設沒有選中狀態
    }
}

void LightManager::renderLightMarkers(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    // 確保有位置光源需要渲染
    bool hasPositionalLights = false;
    for (const auto& light : lights) {
        if (light.getType() == LightType::POSITIONAL && light.isEnabled()) {
            hasPositionalLights = true;
            break;
        }
    }
    
    if (!hasPositionalLights) {
        return; // 沒有位置光源，不需要渲染
    }
    
    // 檢查 VAO 是否有效
    if (lightMarkerVAO == 0) {
        std::cerr << "Light marker VAO not initialized" << std::endl;
        return;
    }
    
    // 保存當前的 OpenGL 狀態
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    
    // 確保深度測試開啟（光源標記應該遵循深度）
    glEnable(GL_DEPTH_TEST);
    
    // 禁用 blend，確保光源標記是純白色
    glDisable(GL_BLEND);
      // 確保使用正確的 shader
    shader.use();
    
    // 設置白色顏色
    glUniform3f(glGetUniformLocation(shader.ID, "objColor"), 1.0f, 1.0f, 1.0f);
    glUniform1i(glGetUniformLocation(shader.ID, "isRoom"), 0);
    glUniform1i(glGetUniformLocation(shader.ID, "isAABB"), 0);
    glUniform1i(glGetUniformLocation(shader.ID, "isReflection"), 0);
    
    // 渲染所有位置光源的標記
    for (const auto& light : lights) {
        if (light.getType() == LightType::POSITIONAL && light.isEnabled()) {
            glm::mat4 lightModelMat = glm::translate(glm::mat4(1.0f), light.getPosition());
            glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(lightModelMat));
            glUniformMatrix4fv(glGetUniformLocation(shader.ID, "viewMat"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projMat"), 1, GL_FALSE, glm::value_ptr(projection));
            
            glBindVertexArray(lightMarkerVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
    
    // 恢復之前的狀態
    if (!depthTestEnabled) {
        glDisable(GL_DEPTH_TEST);
    }
    if (blendEnabled) {
        glEnable(GL_BLEND);
    }
    
    // 確保解除 VAO 綁定
    glBindVertexArray(0);
}

std::vector<glm::vec3> LightManager::getPositionalLightPositions() const {
    std::vector<glm::vec3> positions;
    for (const auto& light : lights) {
        if (light.getType() == LightType::POSITIONAL && light.isEnabled()) {
            positions.push_back(light.getPosition());
        }
    }
    return positions;
}
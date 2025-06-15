#include "Light.h"
#include <imgui.h>  // Comment out if ImGui is not available
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

// 光源頂點
static float lightMarkerVertices[] = {
    // 前面
    -0.1f, -0.1f, -0.1f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
     0.1f, -0.1f, -0.1f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,
     0.1f,  0.1f, -0.1f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
     0.1f,  0.1f, -0.1f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
    -0.1f,  0.1f, -0.1f,  0.0f, 1.0f,  0.0f, 0.0f, -1.0f,
    -0.1f, -0.1f, -0.1f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,

    // 後面
    -0.1f, -0.1f,  0.1f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
     0.1f, -0.1f,  0.1f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
     0.1f,  0.1f,  0.1f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
     0.1f,  0.1f,  0.1f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
    -0.1f,  0.1f,  0.1f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
    -0.1f, -0.1f,  0.1f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,

    // 左面
    -0.1f,  0.1f,  0.1f,  1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    -0.1f,  0.1f, -0.1f,  1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -0.1f, -0.1f, -0.1f,  0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -0.1f, -0.1f, -0.1f,  0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -0.1f, -0.1f,  0.1f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    -0.1f,  0.1f,  0.1f,  1.0f, 0.0f, -1.0f, 0.0f, 0.0f,

    // 右面
     0.1f,  0.1f,  0.1f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
     0.1f,  0.1f, -0.1f,  1.0f, 1.0f,  1.0f, 0.0f, 0.0f,
     0.1f, -0.1f, -0.1f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f,
     0.1f, -0.1f, -0.1f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f,
     0.1f, -0.1f,  0.1f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
     0.1f,  0.1f,  0.1f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,

    // 底面
    -0.1f, -0.1f, -0.1f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,
     0.1f, -0.1f, -0.1f,  1.0f, 1.0f,  0.0f, -1.0f, 0.0f,
     0.1f, -0.1f,  0.1f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
     0.1f, -0.1f,  0.1f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
    -0.1f, -0.1f,  0.1f,  0.0f, 0.0f,  0.0f, -1.0f, 0.0f,
    -0.1f, -0.1f, -0.1f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,

    // 頂面
    -0.1f,  0.1f, -0.1f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f,
     0.1f,  0.1f, -0.1f,  1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
     0.1f,  0.1f,  0.1f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
     0.1f,  0.1f,  0.1f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
    -0.1f,  0.1f,  0.1f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
    -0.1f,  0.1f, -0.1f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f
};

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
    // 更新旋轉角度
    rotation.x = glm::degrees(asin(-direction.y));
    rotation.y = glm::degrees(atan2(direction.x, direction.z));
    rotation.z = 0.0f;
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

glm::vec3 Light::getRotation() const {
    return rotation;
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
    
    if (ImGui::CollapsingHeader(lightName.c_str())) {
        ImGui::Checkbox("Enabled", &enabled);
        
        // 所有光源都可以調整位置
        if (ImGui::SliderFloat3("Position", glm::value_ptr(position), 0.0f, 10.0f)) {
            // 位置改變時不需要特殊處理
        }
        
        // 所有光源都可以調整旋轉
        if (ImGui::SliderFloat3("Rotation (degrees)", glm::value_ptr(rotation), -180.0f, 180.0f)) {
            if (type == LightType::DIRECTIONAL) {
                updateDirectionFromRotation();
            }
        }
        
        if (type == LightType::POSITIONAL) {
            ImGui::Text("Type: Positional Light");
        } else {
            ImGui::Text("Type: Directional Light");
            ImGui::Text("Direction: (%.2f, %.2f, %.2f)", direction.x, direction.y, direction.z);
        }
        
        ImGui::ColorEdit3("Color", glm::value_ptr(color));
        ImGui::SliderFloat("Intensity", &intensity, 0.0f, 3.0f);
    }
    
    if (isSelected) {
        ImGui::PopStyleColor();
    }
}

// LightManager 類實現
LightManager::LightManager() : lightMarkerVAO(0), lightMarkerVBO(0), selectedLightIndex(-1) {
    // 添加預設光源
    addPositionalLight(glm::vec3(2.0f, 8.0f, 2.0f), glm::vec3(0.5f, 0.5f, 0.5f));
    addDirectionalLight(glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(0.2f, 0.7f, 0.9f));
    
    // 預設選中第一個光源
    selectedLightIndex = 0;
    
    // 不在構造函數中初始化 OpenGL 緩衝區，稍後手動初始化
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

void LightManager::setSelectedLight(int index) {
    if (index >= -1 && index < static_cast<int>(lights.size())) {
        selectedLightIndex = index;
    }
}

int LightManager::getSelectedLight() const {
    return selectedLightIndex;
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
    
    // 光源選擇區域
    ImGui::Text("Select Light to Edit:");
    for (size_t i = 0; i < lights.size(); ++i) {
        std::string buttonLabel;
        if (lights[i].getType() == LightType::POSITIONAL) {
            buttonLabel = "Positional " + std::to_string(i + 1);
        } else {
            buttonLabel = "Directional " + std::to_string(i + 1);
        }
        
        if (static_cast<int>(i) == selectedLightIndex) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 0.8f)); // 綠色表示選中
        }
        
        if (ImGui::Button(buttonLabel.c_str(), ImVec2(120, 0))) {
            selectedLightIndex = static_cast<int>(i);
        }
        
        if (static_cast<int>(i) == selectedLightIndex) {
            ImGui::PopStyleColor();
        }
        
        if (i < lights.size() - 1) {
            ImGui::SameLine();
        }
    }
    
    ImGui::Separator();
    
    // 顯示選中光源的詳細控制
    if (selectedLightIndex >= 0 && selectedLightIndex < static_cast<int>(lights.size())) {
        std::string lightName;
        if (lights[selectedLightIndex].getType() == LightType::POSITIONAL) {
            lightName = "Positional Light " + std::to_string(selectedLightIndex + 1);
        } else {
            lightName = "Directional Light " + std::to_string(selectedLightIndex + 1);
        }
        lights[selectedLightIndex].renderImGuiControls(lightName, true);
    } else {
        ImGui::Text("No light selected");
    }
}

void LightManager::renderLightMarkers(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    // 如果緩衝區未初始化，則不渲染
    if (lightMarkerVAO == 0) return;
    
    // 渲染所有光源的標記（不論類型）
    for (size_t i = 0; i < lights.size(); ++i) {
        const auto& light = lights[i];
        if (light.isEnabled()) {
            // 根據光源類型設置不同的顏色
            if (light.getType() == LightType::POSITIONAL) {
                // 位置光源顯示為白色
                glUniform3f(glGetUniformLocation(shader.ID, "objColor"), 1.0f, 1.0f, 1.0f);
            } else {
                // 方向光源顯示為黃色
                glUniform3f(glGetUniformLocation(shader.ID, "objColor"), 1.0f, 1.0f, 0.0f);
            }
            
            // 選中的光源顯示為綠色
            if (static_cast<int>(i) == selectedLightIndex) {
                glUniform3f(glGetUniformLocation(shader.ID, "objColor"), 0.0f, 1.0f, 0.0f);
            }
              glm::mat4 lightModelMat = glm::translate(glm::mat4(1.0f), light.getPosition());
            glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(lightModelMat));
            glUniformMatrix4fv(glGetUniformLocation(shader.ID, "viewMat"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projMat"), 1, GL_FALSE, glm::value_ptr(projection));
            
            glBindVertexArray(lightMarkerVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
}

std::vector<glm::vec3> LightManager::getPositionalLightPositions() const {
    std::vector<glm::vec3> positions;
    for (const auto& light : lights) {
        if (light.isEnabled()) {  // 返回所有啟用光源的位置，不論類型
            positions.push_back(light.getPosition());
        }
    }
    return positions;
}
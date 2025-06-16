#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include "Shader.h"

enum class LightType {
    POSITIONAL,
    DIRECTIONAL
};

class Light {
public:
    LightType type;
    glm::vec3 position;      // 位置光源使用
    glm::vec3 direction;     // 方向光源使用
    glm::vec3 rotation;      // 旋轉角度 (度)，方向光源使用
    glm::vec3 color;
    float intensity;
    bool enabled;

    // 構造函數
    Light(LightType lightType, glm::vec3 pos_or_dir, glm::vec3 lightColor = glm::vec3(1.0f), float lightIntensity = 1.0f);
      // 設置光源屬性
    void setPosition(const glm::vec3& pos);
    void setDirection(const glm::vec3& dir);
    void setRotation(const glm::vec3& rot);
    void setColor(const glm::vec3& lightColor);
    void setIntensity(float lightIntensity);
    void setEnabled(bool isEnabled);
    
    // 獲取光源屬性
    glm::vec3 getPosition() const;
    glm::vec3 getDirection() const;
    glm::vec3 getRotation() const;
    glm::vec3 getColor() const;
    float getIntensity() const;bool isEnabled() const;
    LightType getType() const;
    
    // 更新方向基於旋轉
    void updateDirectionFromRotation();
    
    // 應用光源到 shader
    void applyToShader(Shader& shader, const std::string& uniformBaseName) const;
      // ImGui 控制界面
    void renderImGuiControls(const std::string& lightName, bool isSelected);
};

class LightManager {
private:
    std::vector<Light> lights;
    unsigned int lightMarkerVAO, lightMarkerVBO;
    
public:
    LightManager();
    ~LightManager();
    
    // 初始化光源標記的渲染資源
    void initLightMarkerBuffers();
    
    // 添加光源
    void addLight(const Light& light);
    void addPositionalLight(const glm::vec3& position, const glm::vec3& color = glm::vec3(1.0f));
    void addDirectionalLight(const glm::vec3& direction, const glm::vec3& color = glm::vec3(1.0f));
    
    // 獲取光源
    Light& getLight(size_t index);
    const Light& getLight(size_t index) const;
    size_t getLightCount() const;
    
    // 應用所有光源到 shader
    void applyAllLightsToShader(Shader& shader) const;
    
    // ImGui 控制界面
    void renderImGuiControls();
    
    // 渲染光源標記
    void renderLightMarkers(Shader& shader, const glm::mat4& view, const glm::mat4& projection);
    
    // 獲取所有位置光源的位置（用於渲染光源標記）
    std::vector<glm::vec3> getPositionalLightPositions() const;
};
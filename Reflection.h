#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"

class ReflectionRenderer {
public:
    ReflectionRenderer();
    ~ReflectionRenderer();

    // 初始化反射系統
    bool initialize();
    
    // 設置反射平面（y = planeY）
    void setReflectionPlane(float planeY);
    
    // 啟用/禁用反射
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }
    
    // 設置反射透明度
    void setReflectionAlpha(float alpha) { m_reflectionAlpha = alpha; }
    
    // 開始反射渲染（設置 stencil buffer 和反射矩陣）
    void beginReflectionPass(const glm::mat4& viewMat, const glm::mat4& projMat);
    
    // 結束反射渲染（恢復正常狀態）
    void endReflectionPass();
    
    // 獲取反射矩陣
    glm::mat4 getReflectionMatrix() const { return m_reflectionMatrix; }
    
    // 檢查是否在反射渲染中
    bool isInReflectionPass() const { return m_inReflectionPass; }

private:
    bool m_enabled;
    float m_reflectionPlane;  // y = m_reflectionPlane
    float m_reflectionAlpha;
    glm::mat4 m_reflectionMatrix;
    bool m_inReflectionPass;
    
    // OpenGL 狀態保存
    GLboolean m_originalDepthMask;
    GLint m_originalCullFaceMode;
    GLboolean m_originalCullFace;
    GLboolean m_originalBlend;
    GLint m_originalBlendSrc, m_originalBlendDst;
    
    void setupReflectionStencil();
    void setupReflectionState();
    void restoreOriginalState();
};

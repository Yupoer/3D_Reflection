#include "Reflection.h"
#include <iostream>

ReflectionRenderer::ReflectionRenderer() 
    : m_enabled(true)
    , m_reflectionPlane(0.0f)  // 預設反射平面在 y = 0
    , m_reflectionAlpha(0.6f)
    , m_inReflectionPass(false)
{
    // 創建反射矩陣：沿 Y 軸鏡像
    m_reflectionMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));
}

ReflectionRenderer::~ReflectionRenderer() {
}

bool ReflectionRenderer::initialize() {
    // 檢查是否支援 stencil buffer
    GLint stencilBits;
    glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
    if (stencilBits == 0) {
        std::cout << "Warning: No stencil buffer available for reflection rendering" << std::endl;
        return false;
    }
    
    std::cout << "Reflection system initialized successfully" << std::endl;
    return true;
}

void ReflectionRenderer::setReflectionPlane(float planeY) {
    m_reflectionPlane = planeY;
    
    // 更新反射矩陣：先平移到原點，鏡像，再平移回去
    glm::mat4 translate1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -planeY, 0.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));
    glm::mat4 translate2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, planeY, 0.0f));
    
    m_reflectionMatrix = translate2 * scale * translate1;
}

void ReflectionRenderer::beginReflectionPass(const glm::mat4& viewMat, const glm::mat4& projMat) {
    if (!m_enabled) return;
    
    m_inReflectionPass = true;
    
    // 保存原始 OpenGL 狀態
    glGetBooleanv(GL_DEPTH_WRITEMASK, &m_originalDepthMask);
    glGetIntegerv(GL_CULL_FACE_MODE, &m_originalCullFaceMode);
    glGetBooleanv(GL_CULL_FACE, &m_originalCullFace);
    glGetBooleanv(GL_BLEND, &m_originalBlend);
    glGetIntegerv(GL_BLEND_SRC, &m_originalBlendSrc);
    glGetIntegerv(GL_BLEND_DST, &m_originalBlendDst);
    
    setupReflectionStencil();
    setupReflectionState();
}

void ReflectionRenderer::endReflectionPass() {
    if (!m_enabled || !m_inReflectionPass) return;
    
    restoreOriginalState();
    m_inReflectionPass = false;
}

void ReflectionRenderer::setupReflectionStencil() {
    // 第一步：在 stencil buffer 中標記反射平面區域
    
    // 清除 stencil buffer
    glClear(GL_STENCIL_BUFFER_BIT);
    
    // 設置 stencil 測試：總是通過，並將通過的像素設為 1
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    
    // 禁用深度寫入，只更新 stencil buffer
    glDepthMask(GL_FALSE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    
    // 這裡應該渲染反射平面的幾何體來標記 stencil
    // 由於我們的反射平面是房間的地板，我們需要在主渲染循環中處理
    
    // 重新啟用顏色和深度寫入
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    
    // 設置 stencil 測試：只在 stencil 值為 1 的地方渲染
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

void ReflectionRenderer::setupReflectionState() {
    // 啟用混合，讓反射有透明效果
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 反轉面剔除（因為我們翻轉了 Y 軸）
    if (m_originalCullFace) {
        if (m_originalCullFaceMode == GL_BACK) {
            glCullFace(GL_FRONT);
        } else if (m_originalCullFaceMode == GL_FRONT) {
            glCullFace(GL_BACK);
        }
    }
}

void ReflectionRenderer::restoreOriginalState() {
    // 恢復原始狀態
    glDepthMask(m_originalDepthMask);
    
    if (m_originalCullFace) {
        glEnable(GL_CULL_FACE);
        glCullFace(m_originalCullFaceMode);
    } else {
        glDisable(GL_CULL_FACE);
    }
    
    if (m_originalBlend) {
        glEnable(GL_BLEND);
        glBlendFunc(m_originalBlendSrc, m_originalBlendDst);
    } else {
        glDisable(GL_BLEND);
    }
    
    // 禁用 stencil 測試
    glDisable(GL_STENCIL_TEST);
}

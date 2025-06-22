#pragma once
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION 
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"
#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "ball.h"
#include "vector"
#include "irregular.h"
#include "room.h"
#include "object.h"
#include <thread>
#include <chrono>
#include "physicManager.h"
#include "Shadow.h"
#include "light.h"
#include "Reflection.h"

// AABB 線框的頂點數據
float aabbVertices[] = {
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    // 後面
    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f
};

unsigned int aabbIndices[] = {
    // 前面
    0, 1, 1, 2, 2, 3, 3, 0,
    // 後面
    4, 5, 5, 6, 6, 7, 7, 4,
    // 連接前後
    0, 4, 1, 5, 2, 6, 3, 7
};

#pragma region Input Declare
// Forward declare apply_impulse_to_objects function if it's defined later or in another file
// For now, we will define a simple version or handle input directly in processInput
void apply_external_force_to_objects(Object& ball, Object& irregular, bool& shouldApply);

// Forward declare rendering functions
void renderScene(Shader* shader, const glm::mat4& viewMat, const glm::mat4& projMat, 
                const glm::mat4& modelMat, const Camera& camera, LightManager& lightManager,
                unsigned int roomVAO, unsigned int irregularVAO, int irregularCount,
                unsigned int TexBufferA, const Object& irregularObj, bool isReflection = false,
                const glm::mat4* reflectionMatrix = nullptr);

void renderFloorStencil(Shader* shader, const glm::mat4& viewMat, const glm::mat4& projMat,
                       const glm::mat4& modelMat, unsigned int roomVAO);

void renderReflectedObjects(Shader* shader, const glm::mat4& viewMat, const glm::mat4& projMat,
                           const Camera& camera, LightManager& lightManager,
                           unsigned int irregularVAO, int irregularCount,
                           const Object& irregularObj, const glm::mat4& reflectionMatrix);

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    // Placeholder for applying force - e.g., press 'J' to apply impulse
    // static bool j_key_pressed = false;
    // if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS && !j_key_pressed) {
    //     j_key_pressed = true;
    //     // apply_external_force_to_objects(ballObj, irregularObj, true); // This needs ballObj and irregularObj to be accessible
    // }
    // if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE) {
    //    j_key_pressed = false;
    // }
}
#pragma endregion

unsigned int LoadImageToGPU(const char* filename, GLint internalFormat, GLenum format, int textureSlot) {
    unsigned int TexBuffer;
    glGenTextures(1, &TexBuffer);
    glActiveTexture(GL_TEXTURE0 + textureSlot);
    glBindTexture(GL_TEXTURE_2D, TexBuffer);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        printf("Texture %s loaded successfully: %d x %d\n", filename, width, height);
    }
    else {
        printf("Failed to load texture: %s\n", stbi_failure_reason());
    }
    stbi_image_free(data);
    return TexBuffer;
}

// Time tracking for physics
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Add physics controls for GUI
bool pausePhysics = false;
float gravityStrength = 9.8f;
bool resetBall = false;
bool resetIrregular = false;
bool showShadows = true; // Toggle for shadows
bool showReflections = true; // Toggle for reflections

// 修改 Object 初始化，加入阻尼參數
Object irregularObj(
    glm::vec3(irregularMin[0], irregularMin[1], irregularMin[2]),  // localMin
    glm::vec3(irregularMax[0], irregularMax[1], irregularMax[2]),  // localMax
    glm::vec3(7.0f, 2.0f, 7.0f),                                   // startPos - Y raised to 2.0f to be above floor
    1.0f,                                                          // 質量
    0.8f,                                                          // 摩擦係數
    0.3f,                                                          // 彈性係數 (was 0.2f, now 0.3f for irr)
    0.75f                                                          // 阻尼係數 (was 0.85f, now 0.75f for irr - larger drag)
);

PhysicManager physicManager(gravityStrength);

#pragma region Helper Function to Create VAO and VBO
void setupModelBuffers(unsigned int& VAO, unsigned int& VBO, const float* vertices, int vertexCount) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertexCount * 8, vertices, GL_STATIC_DRAW);
    
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
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
#pragma endregion

int main() {
    // 設置初始旋轉（繞 Y 軸旋轉 45 度）
    glm::quat initialRotation = glm::angleAxis(glm::radians(45.0f), glm::vec3(1.0f, 1.0f, 0.0f));
    irregularObj.initRotation = initialRotation;
    irregularObj.rotation = initialRotation;

    #pragma region Open a Window
        if (!glfwInit()) {
            printf("Failed to initialize GLFW\n");
            return -1;
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_STENCIL_BITS, 8);  // 啟用 stencil buffer
        
        GLFWwindow* window = glfwCreateWindow(1600, 1200, "3D render", NULL, NULL);
        if (window == NULL) {
            const char* description;
            int code = glfwGetError(&description);
            printf("GLFW Error %d: %s\n", code, description);
            glfwTerminate();
            return -1;
        }

        glfwMakeContextCurrent(window);
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        
        // 檢查 stencil buffer 是否正確創建
        int stencilBits;
        glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
        printf("Stencil buffer bits: %d\n", stencilBits);
        if (stencilBits == 0) {
            printf("Warning: No stencil buffer available\n");
        }
  

        // init GLEW
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            glfwTerminate();
            return -1;
        }        glViewport(0, 0, 1600, 1200);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);  // 啟用 stencil testing
        
        // 禁用面剔除，這樣在房間內部也能看到牆壁
        glDisable(GL_CULL_FACE);
    #pragma endregion
      #pragma region Init Shadow System
    // 初始化陰影系統
    ShadowRenderer shadowRenderer;
    if (!shadowRenderer.initialize("shadow.vert", "shadow.frag")) {
        printf("Failed to initialize shadow system\n");
    } else {
        printf("Shadow system initialized successfully\n");
    }    shadowRenderer.setGroundPlane(0.0f, 1.0f, 0.0f, 0.0f); // y = 0 平面
    shadowRenderer.setShadowAlpha(0.7f); // 設置陰影更透明，便於調試
    #pragma endregion
    
    #pragma region Init Reflection System
    // 初始化反射系統
    ReflectionRenderer reflectionRenderer;
    if (!reflectionRenderer.initialize()) {
        printf("Failed to initialize reflection system\n");
    } else {
        printf("Reflection system initialized successfully\n");
    }
    reflectionRenderer.setReflectionPlane(0.0f); // 反射平面在 y = 0
    reflectionRenderer.setReflectionAlpha(0.6f); // 設置反射透明度
    #pragma endregion
    
    #pragma region Init Light Manager
    // 初始化光源管理器
    LightManager lightManager;
    lightManager.initLightMarkerBuffers(); // 手動初始化 OpenGL 緩衝區
    #pragma endregion
    
    #pragma region Init ImGui
    // 初始化 ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark(); 
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 400 core"); 
    #pragma endregion

    #pragma region Init Shader Program
    // load vertex and fragment shader
    Shader* myShader = new Shader("vertexShaderSource.vert", "fragmentShaderSource.frag");
    #pragma endregion    // Room AABB 初始化
    AABB roomAABB(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(10.0f, 10.0f, 10.0f));     
    #pragma region Init and load Model to VAO & VBO
    // room VAO & VBO
    unsigned int roomVAO, roomVBO;
    setupModelBuffers(roomVAO, roomVBO, roomVertices, sizeof(roomVertices) / sizeof(roomVertices[0]));    // 為 irregularVertices 物件創建 VAO 和 VBO
    unsigned int irregularVAO, irregularVBO;
    setupModelBuffers(irregularVAO, irregularVBO, irregularVertices, irregularCount);

    // 創建 AABB 線框的 VAO 和 VBO
    unsigned int aabbVAO, aabbVBO, aabbEBO;
    glGenVertexArrays(1, &aabbVAO);
    glGenBuffers(1, &aabbVBO);
    glGenBuffers(1, &aabbEBO);

    glBindVertexArray(aabbVAO);
    glBindBuffer(GL_ARRAY_BUFFER, aabbVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(aabbVertices), aabbVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, aabbEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(aabbIndices), aabbIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    #pragma endregion

    #pragma region Init and Load Texture
    unsigned int TexBufferA;
    unsigned int TexBufferB;
    TexBufferA = LoadImageToGPU("picSource/grid.jpg", GL_RGB, GL_RGB, 0);
    TexBufferB = LoadImageToGPU("picSource/container.jpg", GL_RGB, GL_RGB, 3);
    #pragma endregion  

    #pragma region Init Camera
    glm::vec3 position = { 1.0f, 9.0f, 1.0f };
    glm::vec3 worldup = { 0.0f, 1.0f, 0.0f };
    Camera camera(position, glm::radians(0.0f), glm::radians(0.0f), worldup);
    #pragma endregion

    #pragma region Prepare MVP(model view proj) Matrices
    glm::mat4 viewMat = glm::mat4(1.0f);
    viewMat = camera.GetViewMatrix();

    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 5.0f)); // 將房間中心(5,5,5)，使範圍為(0,0,0)到(10,10,10)

    glm::mat4 projMat = glm::mat4(1.0f);
    // 透視投影（FOV 45 度，寬高比 1600/1200，近裁剪面 0.1，遠裁剪面 100）
    projMat = glm::perspective(glm::radians(60.0f), 1600.0f / 1200.0f, 0.1f, 100.0f);
    #pragma endregion    // Time initialization
    lastFrame = glfwGetTime();    // 確保初始速度為零，以實現初始靜止
    irregularObj.SetVelocity(glm::vec3(0.0f));
    irregularObj.angularVelocity = glm::vec3(0.0f);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        processInput(window);
              // Clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        #pragma region ImGui Frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Control", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::SetWindowPos(ImVec2(10, 10));
        ImGui::SetWindowSize(ImVec2(300, 400));

        ImGui::Text("Adjust Main Camera Position (Top-Left View)");
        ImGui::SliderFloat3("Camera Position", &camera.Position[0], -10.0f, 15.0f);
        viewMat = camera.GetViewMatrix();

        ImGui::Text("Adjust Camera Pitch and Yaw");
        float pitch_deg = glm::degrees(camera.Pitch);
        float yaw_deg = glm::degrees(camera.Yaw);
        bool camera_updated = false;
        if (ImGui::SliderFloat("Pitch", &pitch_deg, -89.0f, 89.0f)) {
            camera.Pitch = glm::radians(pitch_deg);
            camera_updated = true;
        }
        if (ImGui::SliderFloat("Yaw", &yaw_deg, -180.0f, 180.0f)) {
            camera.Yaw = glm::radians(yaw_deg);
            camera_updated = true;
        }
        if (camera_updated) {
            camera.UpdateCameraVectors();
            viewMat = camera.GetViewMatrix();        }        // 使用 LightManager 的 GUI 控制
        lightManager.renderImGuiControls();
          ImGui::Separator();
        ImGui::Text("Shadow Controls");
        ImGui::Checkbox("Show Shadows", &showShadows);
        
        ImGui::Separator();
        ImGui::Text("Reflection Controls");
        ImGui::Checkbox("Show Reflections", &showReflections);
        reflectionRenderer.setEnabled(showReflections);
        if (showReflections) {
            float reflectionAlpha = 0.6f;
            if (ImGui::SliderFloat("Reflection Alpha", &reflectionAlpha, 0.1f, 1.0f)) {
                reflectionRenderer.setReflectionAlpha(reflectionAlpha);
            }
        }
        
        ImGui::Separator();
        ImGui::Text("AABB Controls");
        bool showAABB = AABB::GetShowCollisionVolumes();
        if (ImGui::Checkbox("Show AABB Wireframe", &showAABB)) {
            AABB::SetShowCollisionVolumes(showAABB);
        }
        ImGui::Text("Camera Pitch: %.2f degrees", glm::degrees(camera.Pitch));
        ImGui::Text("Camera Yaw: %.2f degrees", glm::degrees(camera.Yaw));
          ImGui::Separator();
        ImGui::Text("Physics Controls");
        ImGui::Checkbox("Pause Physics", &pausePhysics);
        ImGui::SliderFloat("Gravity", &gravityStrength, 0.0f, 20.0f);
        physicManager.gravityAcceleration = gravityStrength;
        ImGui::SliderFloat("Angular Drag", &physicManager.angularDragCoefficient, 0.0f, 5.0f, "%.3f");        if (ImGui::Button("Apply Upward Impulse to All Objects")) {
            glm::vec3 impulse(4.0f, 10.0f, 0.0f); // Impulse vector
            // Apply to irregularObj
            irregularObj.applyImpulse(impulse, irregularObj.getWorldCenterOfMass());
        }
          if (ImGui::Button("Reset All Objects")) {
            resetIrregular = true;
            irregularObj.reset();
        }
        
        ImGui::End();
        #pragma endregion        // 設置視口為整個窗口
        glViewport(0, 0, 1600, 1200);
        myShader->use();
        glUniform3f(glGetUniformLocation(myShader->ID, "ambientColor"), 1.0f, 1.0f, 1.0f);        
        #pragma region Reflection and Scene Rendering
        
        // 第一步：直接渲染正常場景（房間和物件），不使用複雜的 renderScene 函數
        myShader->use();
        
        // 使用 LightManager 應用光源到 shader
        lightManager.applyAllLightsToShader(*myShader);
        glUniform3f(glGetUniformLocation(myShader->ID, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);
        glUniform1i(glGetUniformLocation(myShader->ID, "isReflection"), 0);

        // 渲染房間
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TexBufferA);
        glUniform1i(glGetUniformLocation(myShader->ID, "roomTex"), 0);
        glUniform1i(glGetUniformLocation(myShader->ID, "isRoom"), 1);
        glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
        glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMat));
        glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "projMat"), 1, GL_FALSE, glm::value_ptr(projMat));
        glUniform3f(glGetUniformLocation(myShader->ID, "objColor"), 0.5f, 0.5f, 0.5f);
        glBindVertexArray(roomVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glUniform1i(glGetUniformLocation(myShader->ID, "isRoom"), 0);

        // 渲染不規則物體
        glUniform3f(glGetUniformLocation(myShader->ID, "objColor"), 0.8f, 0.2f, 0.2f);
        glm::mat4 irregularModelMat = glm::translate(glm::mat4(1.0f), irregularObj.GetPosition());
        irregularModelMat = irregularModelMat * glm::mat4_cast(irregularObj.GetRotation());
        glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(irregularModelMat));
        glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMat));
        glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "projMat"), 1, GL_FALSE, glm::value_ptr(projMat));
        glBindVertexArray(irregularVAO);        glDrawArrays(GL_TRIANGLES, 0, irregularCount);
        
        // 第二步：如果啟用反射，渲染反射物件
        if (showReflections && reflectionRenderer.isEnabled()) {
            // Debug: 檢查反射是否被啟用
            static bool debugOnce = true;
            if (debugOnce) {
                printf("Reflection rendering enabled\n");
                debugOnce = false;
            }
            
            // 第二步：設置 stencil buffer 來限制反射區域
            glEnable(GL_STENCIL_TEST);
            
            // 清除 stencil buffer
            glClear(GL_STENCIL_BUFFER_BIT);
            
            // 在 stencil buffer 中標記地板
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            
            // 只更新 stencil，不寫入顏色或深度
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glDepthMask(GL_FALSE);
            
            // 渲染地板到 stencil buffer
            renderFloorStencil(myShader, viewMat, projMat, modelMat, roomVAO);
            
            // 恢復顏色和深度寫入
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDepthMask(GL_TRUE);
              // 第三步：只在地板區域渲染反射物件
            glStencilFunc(GL_EQUAL, 1, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            
            // 啟用混合來讓反射有透明效果
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            // 關鍵修正：修改深度測試設置，讓反射物件能正確顯示在地板上
            glDepthFunc(GL_LEQUAL);  // 改為小於等於，允許相同深度的像素通過
            
            // 暫時啟用面剔除並反轉（只對反射物件）
            bool cullFaceWasEnabled = glIsEnabled(GL_CULL_FACE);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);  // 反轉面剔除
            
            // 渲染反射物件
            glm::mat4 reflectionMatrix = reflectionRenderer.getReflectionMatrix();
            renderReflectedObjects(myShader, viewMat, projMat, camera, lightManager,
                                 irregularVAO, irregularCount, irregularObj, reflectionMatrix);
            
            // 恢復原始狀態
            glDepthFunc(GL_LESS);  // 恢復正常深度測試
            if (!cullFaceWasEnabled) {
                glDisable(GL_CULL_FACE);
            } else {
                glCullFace(GL_BACK);  // 恢復正常面剔除
            }
            
            glDisable(GL_BLEND);
            glDisable(GL_STENCIL_TEST);
        }
        
        #pragma endregion
        
        #pragma region Draw Other Collision Volumes
        // 繪製 OBB
        glUseProgram(myShader->ID);
        OBB::DrawOBB(irregularObj.GetOBB(), myShader->ID, aabbVAO, AABB::GetShowCollisionVolumes());
        
        // Draw the AABB for the room
        AABB::DrawAABB(roomAABB, myShader->ID, aabbVAO, AABB::GetShowCollisionVolumes());
        glUniform1i(glGetUniformLocation(myShader->ID, "isAABB"), 0);
        #pragma endregion                  
          #pragma region Render Shadows
        shadowRenderer.setEnabled(showShadows);
        if (showShadows && shadowRenderer.isEnabled()) {
            // 遍歷所有光源進行陰影投射
            for (size_t i = 0; i < lightManager.getLightCount(); ++i) {
                const Light& light = lightManager.getLight(i);
                if (light.isEnabled()) {
                    // 計算不規則物體的模型矩陣（用於陰影）
                    glm::mat4 irregularModelMat = glm::translate(glm::mat4(1.0f), irregularObj.GetPosition());
                    irregularModelMat = irregularModelMat * glm::mat4_cast(irregularObj.GetRotation());
                    
                    // 渲染不規則物體的陰影
                    shadowRenderer.renderShadow(irregularModelMat, viewMat, projMat, irregularVAO, irregularCount, light);
                }
            }
        }
        #pragma endregion
          #pragma region Render Light Markers (Small white dots for positional lights)
        // 確保使用正確的 shader 並設置正確的 uniforms
        myShader->use();
        glUniform1i(glGetUniformLocation(myShader->ID, "isRoom"), 0);
        glUniform1i(glGetUniformLocation(myShader->ID, "isAABB"), 0);
        
        // 渲染位置光源的小白點標記（只有 Positional 光源才顯示）
        lightManager.renderLightMarkers(*myShader, viewMat, projMat);
        #pragma endregionglBindVertexArray(0);
        
        // 檢查 OpenGL 錯誤 (只在 debug 模式下或需要時啟用)
        #ifdef DEBUG_OPENGL_ERRORS
        {
            GLenum err = glGetError();
            if (err != GL_NO_ERROR) {
                std::cerr << "OpenGL Error: " << err << std::endl;
            }
        }
        #endif
          #pragma region Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());        
        #pragma endregion        // 物理模擬
        std::vector<Object*> objects = { &irregularObj };
        physicManager.pausePhysics = pausePhysics; // 同步暫停狀態
        physicManager.update(objects, roomAABB, deltaTime);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // 限制 FPS
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    // Exit program
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

// 渲染場景函數（用於正常渲染和反射渲染）
void renderScene(Shader* shader, const glm::mat4& viewMat, const glm::mat4& projMat, 
                const glm::mat4& modelMat, const Camera& camera, LightManager& lightManager,
                unsigned int roomVAO, unsigned int irregularVAO, int irregularCount,
                unsigned int TexBufferA, const Object& irregularObj, bool isReflection,
                const glm::mat4* reflectionMatrix) {
    
    shader->use();
    
    // 設置光源
    lightManager.applyAllLightsToShader(*shader);
    glUniform3f(glGetUniformLocation(shader->ID, "ambientColor"), 1.0f, 1.0f, 1.0f);
    glUniform3f(glGetUniformLocation(shader->ID, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);
    
    // 如果是反射渲染，設置透明度
    if (isReflection) {
        glUniform1f(glGetUniformLocation(shader->ID, "reflectionAlpha"), 0.6f);
        glUniform1i(glGetUniformLocation(shader->ID, "isReflection"), 1);
    } else {
        glUniform1i(glGetUniformLocation(shader->ID, "isReflection"), 0);
    }
    
    // 渲染房間（在反射中跳過地板）
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TexBufferA);
    glUniform1i(glGetUniformLocation(shader->ID, "roomTex"), 0);
    glUniform1i(glGetUniformLocation(shader->ID, "isRoom"), 1);
    
    glm::mat4 finalModelMat = modelMat;
    if (isReflection && reflectionMatrix) {
        finalModelMat = (*reflectionMatrix) * modelMat;
    }
    
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(finalModelMat));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMat));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projMat"), 1, GL_FALSE, glm::value_ptr(projMat));
    glUniform3f(glGetUniformLocation(shader->ID, "objColor"), 0.5f, 0.5f, 0.5f);
    
    glBindVertexArray(roomVAO);
    
    if (isReflection) {
        // 反射時只渲染房間的牆壁，不渲染地板
        // 前面 (0-5)
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // 後面 (6-11)  
        glDrawArrays(GL_TRIANGLES, 6, 6);
        // 上面 (12-17)
        glDrawArrays(GL_TRIANGLES, 12, 6);
        // 右面 (24-29)
        glDrawArrays(GL_TRIANGLES, 24, 6);
        // 左面 (30-35)
        glDrawArrays(GL_TRIANGLES, 30, 6);
        // 跳過地板 (18-23)
    } else {
        // 正常渲染房間的所有面
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    
    glUniform1i(glGetUniformLocation(shader->ID, "isRoom"), 0);
    
    // 渲染不規則物體
    glUniform3f(glGetUniformLocation(shader->ID, "objColor"), 0.8f, 0.2f, 0.2f);
    glm::mat4 irregularModelMat = glm::translate(glm::mat4(1.0f), irregularObj.GetPosition());
    irregularModelMat = irregularModelMat * glm::mat4_cast(irregularObj.GetRotation());
    
    if (isReflection && reflectionMatrix) {
        irregularModelMat = (*reflectionMatrix) * irregularModelMat;
    }
    
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(irregularModelMat));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMat));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projMat"), 1, GL_FALSE, glm::value_ptr(projMat));
    
    glBindVertexArray(irregularVAO);
    glDrawArrays(GL_TRIANGLES, 0, irregularCount);
}

// 渲染地板到 stencil buffer
void renderFloorStencil(Shader* shader, const glm::mat4& viewMat, const glm::mat4& projMat,
                       const glm::mat4& modelMat, unsigned int roomVAO) {
    shader->use();
    
    // 設置基本 uniform
    glUniform1i(glGetUniformLocation(shader->ID, "isRoom"), 0);
    glUniform1i(glGetUniformLocation(shader->ID, "isAABB"), 0);
    glUniform1i(glGetUniformLocation(shader->ID, "isReflection"), 0);
    
    // 設置矩陣
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMat));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projMat"), 1, GL_FALSE, glm::value_ptr(projMat));
    
    glBindVertexArray(roomVAO);
    
    // 渲染地板面（下面，法線向下 y = -1）
    // 房間結構：前面(0-5), 後面(6-11), 上面(12-17), 下面(18-23), 右面(24-29), 左面(30-35)
    glDrawArrays(GL_TRIANGLES, 18, 6); // 地板是下面，從索引 18 開始，6個頂點
}

// 渲染反射物件函數
void renderReflectedObjects(Shader* shader, const glm::mat4& viewMat, const glm::mat4& projMat, 
                          const Camera& camera, LightManager& lightManager,
                          unsigned int irregularVAO, int irregularCount,
                          const Object& irregularObj, const glm::mat4& reflectionMatrix) {
    
    shader->use();
    
    // 設置光源
    lightManager.applyAllLightsToShader(*shader);
    glUniform3f(glGetUniformLocation(shader->ID, "ambientColor"), 1.0f, 1.0f, 1.0f);
    glUniform3f(glGetUniformLocation(shader->ID, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);
    
    // 設置反射渲染參數
    glUniform1f(glGetUniformLocation(shader->ID, "reflectionAlpha"), 0.6f);
    glUniform1i(glGetUniformLocation(shader->ID, "isReflection"), 1);
    glUniform1i(glGetUniformLocation(shader->ID, "isRoom"), 0);
    
    // 渲染反射的不規則物體
    glUniform3f(glGetUniformLocation(shader->ID, "objColor"), 0.8f, 0.2f, 0.2f);
      // 計算反射後的模型矩陣，並稍微向上偏移避免 z-fighting
    glm::mat4 irregularModelMat = glm::translate(glm::mat4(1.0f), irregularObj.GetPosition());
    irregularModelMat = irregularModelMat * glm::mat4_cast(irregularObj.GetRotation());
    glm::mat4 reflectedModelMat = reflectionMatrix * irregularModelMat;
    
    // 關鍵修正：將反射物件稍微向上偏移，避免與地板 z-fighting
    glm::mat4 offsetMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.01f, 0.0f));
    reflectedModelMat = offsetMat * reflectedModelMat;
    
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(reflectedModelMat));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMat));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projMat"), 1, GL_FALSE, glm::value_ptr(projMat));
    
    glBindVertexArray(irregularVAO);
    glDrawArrays(GL_TRIANGLES, 0, irregularCount);
}
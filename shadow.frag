#version 330 core

uniform float shadowAlpha;

out vec4 FragColor;

void main() {
    // 實心黑色陰影，使用 shadowAlpha 控制透明度
    FragColor = vec4(0.0, 0.0, 0.0, shadowAlpha);
}

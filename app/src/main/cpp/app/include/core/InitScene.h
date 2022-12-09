#pragma once

#include <glm/glm.hpp>

#include "core/Scene.h"

#include "render/ImageTexture.h"
#include "render/Plane2D.h"
#include "render/Shader.h"

namespace android_slam
{

class InitScene : public Scene
{
public:
    explicit InitScene(App& app, const char* name) noexcept : Scene(app, name) {}
    InitScene(const InitScene&)            = delete;
    InitScene& operator=(const InitScene&) = delete;
    virtual ~InitScene() noexcept          = default;

    void init() override;
    void exit() override;

    void update(float dt) override;
    void drawGui(float dt) override;

private:
    std::unique_ptr<Plane2D>      m_image_painter;  // 用于绘制图像
    std::unique_ptr<ImageTexture> m_image;          // 记录所需绘制的图像
    std::unique_ptr<Shader>       m_image_shader;   // 如何绘制图像的shader

    glm::mat4 m_proj;  // 透视矩阵
    glm::mat4 m_view;  // 视角矩阵

    bool m_initialized = false;  // 用于指示Init界面显示的字符，与真实SLAM线程无关
};

}  // namespace android_slam
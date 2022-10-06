#pragma once

#include <glm/glm.hpp>

#include "core/Scene.h"

#include "render/Plane2D.h"
#include "render/ImageTexture.h"
#include "render/Shader.h"

namespace android_slam
{

    class InitScene : public Scene
    {
    public:
        explicit InitScene(App& app) noexcept
            : Scene(app)
        {}
        InitScene(const InitScene&) = delete;
        InitScene& operator=(const InitScene&) = delete;
        virtual ~InitScene() noexcept = default;

        void init() override;
        void exit() override;

        void update(float dt) override;
        void drawGui(float dt) override;

    private:
        std::unique_ptr<Plane2D> m_image_painter;
        std::unique_ptr<ImageTexture> m_image;
        std::unique_ptr<Shader> m_image_shader;

        glm::mat4 m_proj;
        glm::mat4 m_view;

        bool m_initialized = false;
    };

}
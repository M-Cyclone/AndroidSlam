#pragma once

#include "core/Scene.h"

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
        bool m_initialized = false;
    };

}
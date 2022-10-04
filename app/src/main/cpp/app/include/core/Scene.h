#pragma once

#include <imgui.h>
#include <imgui_impl_android.h>
#include <imgui_impl_opengl3.h>

namespace android_slam
{

    class App;

    class Scene
    {
        friend class App;

    public:
        explicit Scene(App& app) noexcept
            : m_app(app)
        {}
        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;
        virtual ~Scene() noexcept = default;

        virtual void init() = 0;
        virtual void exit() = 0;

        virtual void update(float dt) = 0;
        virtual void drawGui(float dt) = 0;

    protected:
        bool m_show_ui = false;

    protected:
        App& m_app;
    };

}
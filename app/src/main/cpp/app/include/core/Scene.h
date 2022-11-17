#pragma once
#include <string>

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
        explicit Scene(App& app, const char* name) noexcept
            : m_app_ref(app)
            , m_name(name)
        {}
        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;
        virtual ~Scene() noexcept = default;

        const std::string& getName() const { return m_name; }

        virtual void init() = 0;
        virtual void exit() = 0;

        virtual void update(float dt) = 0;
        virtual void drawGui(float dt) = 0;

    protected:
        bool m_show_ui = false;

    protected:
        App& m_app_ref;
        std::string m_name;
    };

}
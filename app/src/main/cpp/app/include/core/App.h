#pragma once
#include <cassert>
#include <string>
#include <memory>
#include <unordered_map>

#include <android_native_app_glue.h>

#include "core/Window.h"
#include "core/Scene.h"
#include "utils/Timer.h"

namespace android_slam
{

    class App
    {
    private:
        static constexpr const char* k_app_name = "Android_SLAM";
        static constexpr const size_t k_max_fps = 20;
        static constexpr const float k_min_frame_time_second = 1.0f / (float)k_max_fps;

    public:
        App(android_app* state) noexcept;
        App(const App&) = delete;
        App& operator=(const App&) = delete;
        ~App() noexcept = default;

        void run();

        Window& getWindow() { return *m_window; }
        void setActiveScene(const std::string& name)
        {
            if(m_scene_map.find(name) == m_scene_map.end()) return;
            m_active_scene = name;
        }

    private:
        void init();
        void exit();
        void update(float dt);

        static void onCmd(android_app* app, int32_t cmd);
        static int32_t onInput(android_app* app, AInputEvent* ie);

    private:
        Timer m_timer;
        std::unique_ptr<Window> m_window;

        std::unordered_map<std::string, std::shared_ptr<Scene>> m_scene_map;
        std::string m_active_scene;

        bool m_show_app_ui = true;

        bool m_running = true;
        bool m_active = false;
    };

}
#pragma once
#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>


#include <android_native_app_glue.h>

#include "core/Scene.h"
#include "core/Window.h"

#include "utils/Log.h"
#include "utils/Timer.h"

namespace android_slam
{

class App
{
private:
    static constexpr const char*  k_app_name             = "Android_SLAM";
    static constexpr const size_t k_max_fps              = 60;                       // 主线程最大帧率
    static constexpr const float k_min_frame_time_second = 1.0f / (float)k_max_fps;  // 主线程最大帧率情况下每帧时间

public:
    explicit App(android_app* state) noexcept;
    App(const App&)            = delete;
    App& operator=(const App&) = delete;
    ~App() noexcept { std::cout.rdbuf(nullptr); }

    void run();

    Window& getWindow() { return *m_window; }
    void    setActiveScene(const std::string& name)
    {
        if (m_scene_map.find(name) == m_scene_map.end()) return;
        m_active_scene            = m_scene_map.at(name);
        m_active_scene->m_show_ui = true;
    }

private:
    void init();
    void exit();
    void update(float dt);

    // 触发安卓应用状态机更新时的回调函数
    static void onCmd(android_app* app, int32_t cmd);
    // 触发安卓输入事件时的回调函数
    static int32_t onInput(android_app* app, AInputEvent* ie);

public:
    float m_last_process_delta_time = 1.0f;  // SLAM线程上一帧的处理时长

private:
    AndroidCOutBuffer m_cout_buffer;  // 用于给std::cout重定向

    Timer                   m_timer;   // 使用chrono库的计时器
    std::unique_ptr<Window> m_window;  // 安卓与EGL的窗口句柄

    // 场景维护，比较粗暴地实现了不同界面的切换功能
    std::unordered_map<std::string, std::shared_ptr<Scene>> m_scene_map;
    std::shared_ptr<Scene>                                  m_active_scene;

    bool m_show_app_ui = true;  // 是否显示APP全局控制UI

    bool m_running = true;   // APP是否在运行
    bool m_active  = false;  // APP是否激活SLAM
};

}  // namespace android_slam
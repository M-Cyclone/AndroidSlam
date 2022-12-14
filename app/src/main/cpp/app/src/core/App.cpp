#include "core/App.h"
#include <atomic>
#include <chrono>
#include <thread>


#include <glm/glm.hpp>

#include <imgui.h>
#include <imgui_impl_android.h>
#include <imgui_impl_opengl3.h>

#include "core/InitScene.h"
#include "core/SlamScene.h"

#include "utils/AssetManager.h"

#include "render/SensorTexture.h"

namespace android_slam
{

// 存为全局句柄方便使用
android_app* g_state;

App::App(android_app* state) noexcept
{
    static std::atomic_bool is_init = false;
    assert((!is_init) && "App should only have one instance.");

    // 绑定各参数
    g_state             = state;
    state->userData     = this;
    state->onAppCmd     = App::onCmd;
    state->onInputEvent = App::onInput;

    std::cout.rdbuf(&m_cout_buffer);

    is_init = true;
    SensorTexture::registerFunctions();
}

void App::run()
{
    // 更新计时器
    m_timer.mark();

    // 每帧更新
    while (m_running)
    {
        // 处理帧间的事件
        {
            int32_t              event;
            android_poll_source* source;

            if (ALooper_pollAll((m_window != nullptr ? 1 : 0), nullptr, &event, reinterpret_cast<void**>(&source)) >= 0)
            {
                if (source)
                {
                    source->process(g_state, source);
                }
            }
        }

        // 不活跃时，仅保持60FPS更新，但不做逻辑计算
        if (!m_active)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        // 更新帧逻辑，真正调用update函数
        float dt = m_timer.mark();
        if (dt < k_min_frame_time_second)
        {
            std::this_thread::sleep_for(Timer::Duration(k_min_frame_time_second - dt));
            dt = k_min_frame_time_second;
        }
        update(dt);
    }
}

// App对象初始化
void App::init()
{
    // 绑定Asset Manager
    AssetManager::set(g_state->activity->assetManager);


    // 创建窗口
    int32_t wnd_width  = ANativeWindow_getWidth(g_state->window);
    int32_t wnd_height = ANativeWindow_getHeight(g_state->window);
    m_window           = std::make_unique<Window>(g_state->window, wnd_width, wnd_height, App::k_app_name);


    // ImGui初始化
    {
        // 字体大小
        float font_size = (float)wnd_height * (1.0f / 20);

        IMGUI_CHECKVERSION();

        // 创建ImGui上下文，并设置为黑色风格
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        // ImGui后端使用Android和OpenGL3的实现版本，绑定对应参数
        ImGui_ImplAndroid_Init(g_state->window);
        ImGui_ImplOpenGL3_Init("#version 300 es");

        // 设置ImGui不输出界面初始化布局文件
        ImGuiIO& io    = ImGui::GetIO();
        io.IniFilename = nullptr;

        // 从assets文件夹找到simhei字体，用于中文渲染
        char*  ttf_data;
        size_t ttf_size;
        {
            AAsset* asset = AAssetManager_open(AssetManager::get(), "fonts/simhei.ttf", AASSET_MODE_BUFFER);
            assert(asset && "[Android Slam Shader Info] Failed to open shader file.");

            ttf_size  = AAsset_getLength(asset);
            auto data = (const char*)AAsset_getBuffer(asset);

            // Don't need to free this memory because ImGui will release it when its context being destroyed.
            ttf_data = (char*)malloc(ttf_size);
            memcpy(ttf_data, data, ttf_size);

            AAsset_close(asset);
        }

        // 绑定字体
        auto fonts = io.Fonts;
        fonts->AddFontFromMemoryTTF(ttf_data,
                                    (int)ttf_size,
                                    font_size,
                                    nullptr,
                                    fonts->GetGlyphRangesChineseSimplifiedCommon());
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // 启用docking

        // 安卓屏幕较小，整体放大3倍
        ImGui::GetStyle().ScaleAllSizes(3.0f);
    }


    // 初始化各场景
    {
        m_scene_map.emplace(std::string("Init"), std::make_shared<InitScene>(*this, u8"系统"));
        m_scene_map.emplace(std::string("Slam"), std::make_shared<SlamScene>(*this, u8"SLAM内核"));

        m_scene_map.at("Init")->init();
        m_scene_map.at("Slam")->init();

        setActiveScene("Init");
    }


    // 完成初始化，设置active为true
    m_active = true;
}

void App::exit()
{
    // 退出程序，设置active为false
    m_active = false;


    m_scene_map.at("Slam")->exit();
    m_scene_map.at("Init")->exit();


    // 关闭ImGui
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplAndroid_Shutdown();
        ImGui::DestroyContext();
    }


    // 关闭窗口，主要是清楚EGL对象
    m_window.reset(nullptr);
}

// 主线程每帧更新对应的函数
void App::update(float dt)
{
    // 当前场景逻辑更新
    m_active_scene->update(dt);


    // 创建新的ImGui界面渲染帧
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();

    // 设置ImGui在全界面开启docking
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    // ImGui组件设置
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(u8"组件"))
        {
            if (ImGui::MenuItem(u8"状态"))
            {
                m_show_app_ui = true;
            }

            if (ImGui::MenuItem(m_active_scene->getName().c_str()))
            {
                m_active_scene->m_show_ui = true;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (m_show_app_ui)
    {
        if (ImGui::Begin(u8"应用", &m_show_app_ui))
        {
            ImGui::Text("App  FPS %.2f.", 1.0f / dt);
            ImGui::Text("SLAM FPS %.2f.", 1.0f / m_last_process_delta_time);
        }
        ImGui::End();
    }

    if (m_active_scene->m_show_ui)
    {
        ImGui::Begin(m_active_scene->getName().c_str(), &m_active_scene->m_show_ui);
        {
            m_active_scene->drawGui(dt);
        }
        ImGui::End();
    }


    // 渲染ImGui结果
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


    // 更新交换链，展示图像
    m_window->swapBuffers();
}

void App::onCmd(android_app* app, int32_t cmd)
{
    App& instance = *static_cast<App*>(app->userData);

    /*
     * In android, the event is trigger by the following order:
     * Create -> Start -> Resume -> INIT WINDOW
     * ...
     * TERM WINDOW -> Stop -> Destroy
     */

    switch (cmd)
    {
    case APP_CMD_INIT_WINDOW:
    {
        instance.init();

        std::cout << "App window initialized." << std::endl;

        break;
    }
    case APP_CMD_TERM_WINDOW:
    {
        instance.exit();

        std::cout << "App window terminated." << std::endl;

        break;
    }
    case APP_CMD_WINDOW_RESIZED:
    {
        int32_t new_width  = ANativeWindow_getWidth(app->window);
        int32_t new_height = ANativeWindow_getHeight(app->window);

        if (new_width != instance.m_window->getWidth() || new_height != instance.m_window->getHeight())
        {
            instance.m_window->resize(new_width, new_height);
        }

        std::cout << "App window resized." << std::endl;

        break;
    }
    case APP_CMD_GAINED_FOCUS:
    {
        instance.m_active = true;

        std::cout << "App window gained focus." << std::endl;

        break;
    }
    case APP_CMD_LOST_FOCUS:
    {
        instance.m_active = false;

        std::cout << "App window lost focus." << std::endl;

        break;
    }
    default:
    {
    }
    }
}

int32_t App::onInput(android_app* app, AInputEvent* ie)
{
    return ImGui_ImplAndroid_HandleInputEvent(ie);
}

}  // namespace android_slam
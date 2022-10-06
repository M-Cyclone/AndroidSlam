#include "core/App.h"
#include <thread>
#include <chrono>
#include <atomic>

#include <glm/glm.hpp>

#include <imgui.h>
#include <imgui_impl_android.h>
#include <imgui_impl_opengl3.h>

#include "core/InitScene.h"
#include "core/SlamScene.h"

#include "utils/Log.h"
#include "utils/AssetManager.h"

#include "render/SensorTexture.h"

namespace android_slam
{

    android_app* g_state;

    App::App(android_app* state) noexcept
    {
        static std::atomic_bool is_init = false;
        assert((!is_init) && "App should only have one instance.");

        g_state = state;
        state->userData = this;
        state->onAppCmd = App::onCmd;
        state->onInputEvent = App::onInput;

        SensorTexture::registerFunctions();
    }

    void App::run()
    {
        m_timer.mark();
        while (m_running)
        {
            // Handle events.
            {
                int32_t event;
                android_poll_source* source;

                if (ALooper_pollAll((m_window != nullptr ? 1 : 0), nullptr, &event, reinterpret_cast<void**>(&source)) >= 0)
                {
                    if (source)
                    {
                        source->process(g_state, source);
                    }
                }
            }

            if (!m_active)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                continue;
            }

            float dt = m_timer.mark();
            if (dt < k_min_frame_time_second)
            {
                std::this_thread::sleep_for(Timer::Duration(k_min_frame_time_second - dt));
                dt = k_min_frame_time_second;
            }
            update(dt);
        }
    }

    void App::init()
    {
        AssetManager::set(g_state->activity->assetManager);

        int32_t wnd_width = ANativeWindow_getWidth(g_state->window);
        int32_t wnd_height = ANativeWindow_getHeight(g_state->window);
        m_window = std::make_unique<Window>( g_state->window
                                           , wnd_width
                                           , wnd_height
                                           , App::k_app_name
                                           );


        // ImGui
        {
            float font_size = (float)wnd_height * (1.0f / 20);


            char* ttf_data;
            size_t ttf_size;
            {
                AAsset* asset = AAssetManager_open(AssetManager::get(), "fonts/simhei.ttf", AASSET_MODE_BUFFER);
                assert(asset && "[Android Slam Shader Info] Failed to open shader file.");

                ttf_size = AAsset_getLength(asset);
                auto data = (const char*)AAsset_getBuffer(asset);

                // Don't need to free this memory because ImGui will release it when its context being destroyed.
                ttf_data = (char*)malloc(ttf_size);
                memcpy(ttf_data, data, ttf_size);

                AAsset_close(asset);
            }

            IMGUI_CHECKVERSION();

            ImGui::CreateContext();
            ImGui::StyleColorsDark();

            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = nullptr;

            auto fonts = io.Fonts;
            fonts->AddFontFromMemoryTTF( ttf_data
                                       , (int)ttf_size
                                       , font_size
                                       , nullptr
                                       , fonts->GetGlyphRangesChineseSimplifiedCommon()
                                       );
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

            ImGui_ImplAndroid_Init(g_state->window);
            ImGui_ImplOpenGL3_Init("#version 300 es");
        }


        // Init all scenes.
        {
            m_scene_map.emplace(std::string("Init"), std::make_shared<InitScene>(*this, u8"系统"));
            m_scene_map.emplace(std::string("Slam"), std::make_shared<SlamScene>(*this, u8"SLAM内核"));

            m_scene_map.at("Init")->init();
            m_scene_map.at("Slam")->init();

            setActiveScene("Init");
        }


        m_active = true;
    }

    void App::exit()
    {
        m_active = false;


        m_scene_map.at("Slam")->exit();
        m_scene_map.at("Init")->exit();


        // ImGui
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplAndroid_Shutdown();
            ImGui::DestroyContext();
        }


        m_window.reset(nullptr);
    }

    void App::update(float dt)
    {
        m_active_scene->update(dt);


        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame();
        ImGui::NewFrame();


        ImGui::DockSpaceOverViewport( ImGui::GetMainViewport()
                                    , ImGuiDockNodeFlags_PassthruCentralNode
                                    );

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu(u8"组件"))
            {
                if (ImGui::MenuItem(u8"应用"))
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
                ImGui::Text("Current FPS %.2f.", 1.0f / dt);
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


        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        m_window->swapBuffers();

        DEBUG_INFO("[Android Slam App Info] Current FPS: %.2f frame per second.", 1.0f / dt);
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

                DEBUG_INFO("App window initialized.");

                break;
            }
            case APP_CMD_TERM_WINDOW:
            {
                instance.exit();

                DEBUG_INFO("App window terminated.");

                break;
            }
            case APP_CMD_WINDOW_RESIZED:
            {
                int32_t new_width = ANativeWindow_getWidth(app->window);
                int32_t new_height = ANativeWindow_getHeight(app->window);

                if (new_width != instance.m_window->getWidth() || new_height != instance.m_window->getHeight())
                {
                    instance.m_window->resize(new_width, new_height);
                }

                DEBUG_INFO("App window resized.");

                break;
            }
            case APP_CMD_GAINED_FOCUS:
            {
                instance.m_active = true;

                DEBUG_INFO("App gained focus.");

                break;
            }
            case APP_CMD_LOST_FOCUS:
            {
                instance.m_active = false;

                DEBUG_INFO("App lost focus.");

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

}
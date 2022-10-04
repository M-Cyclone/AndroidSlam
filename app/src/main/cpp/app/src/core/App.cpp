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

        m_window = std::make_unique<Window>(
        g_state->window, ANativeWindow_getWidth(g_state->window), ANativeWindow_getHeight(g_state->window), App::k_app_name
        );


        // ImGui
        {
            IMGUI_CHECKVERSION();

            ImGui::CreateContext();
            ImGui::StyleColorsDark();

            ImFontConfig font_config{};
            font_config.SizePixels = 50.0f;

            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = nullptr;
            io.Fonts->AddFontDefault(&font_config);
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

            ImGui_ImplAndroid_Init(g_state->window);
            ImGui_ImplOpenGL3_Init("#version 300 es");
        }


        // Init all scenes.
        {
            m_scene_map.emplace(std::string("init"), std::make_shared<InitScene>(*this));
            m_scene_map.emplace(std::string("slam"), std::make_shared<SlamScene>(*this));

            for (auto&&[name, scene]: m_scene_map)
            {
                scene->init();
            }

            m_active_scene = "init";
        }


        m_active = true;
    }

    void App::exit()
    {
        m_active = false;


        for (auto&&[name, scene]: m_scene_map)
        {
            scene->exit();
        }


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
        auto active_scene = m_scene_map.at(m_active_scene);
        active_scene->update(dt);


        glViewport(0, 0, m_window->getWidth(), m_window->getHeight());

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame();
        ImGui::NewFrame();


        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Widget"))
            {
                if (ImGui::MenuItem("App"))
                {
                    m_show_app_ui = true;
                }

                if (ImGui::MenuItem(m_active_scene.c_str()))
                {
                    active_scene->m_show_ui = true;
                }

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if (m_show_app_ui)
        {
            if (ImGui::Begin("App", &m_show_app_ui))
            {
                ImGui::Text("Current FPS %.2f.", 1.0f / dt);
            }
            ImGui::End();
        }

        if (active_scene->m_show_ui)
        {
            ImGui::Begin(m_active_scene.c_str(), &active_scene->m_show_ui);
            {
                active_scene->drawGui(dt);
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
        (void) app;
        return ImGui_ImplAndroid_HandleInputEvent(ie);
    }

}
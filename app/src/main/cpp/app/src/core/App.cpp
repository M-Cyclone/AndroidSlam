#include "core/App.h"
#include <thread>
#include <chrono>
#include <atomic>

#include <glm/glm.hpp>

#include "utils/Log.h"
#include "utils/AssetManager.h"

#include "render/ImageTexture.h"
#include "render/Shader.h"
#include "render/Plane2D.h"

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

                if(ALooper_pollAll((m_window != nullptr ? 1 : 0), nullptr, &event, reinterpret_cast<void**>(&source)) >= 0)
                {
                    if(source)
                    {
                        source->process(g_state, source);
                    }
                }
            }

            if(!m_active)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                continue;
            }

            float dt = m_timer.mark();
            if(dt < k_min_frame_time_second)
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
                g_state->window,
                ANativeWindow_getWidth(g_state->window),
                ANativeWindow_getHeight(g_state->window),
                App::k_app_name
        );


        // Camera image converter.
        m_image_pool = std::make_unique<ImagePool>(
            k_sensor_camera_width,
            k_sensor_camera_height,
            "shader/yuv2rgb.vert",
            "shader/yuv2rgb.frag"
        );


        m_slam_renderer = std::make_unique<SlamRenderer>(45.0f, m_window->getAspectRatio(), 0.1f, 1000.0f);


        {
            DEBUG_INFO("[Android Slam App Info] Starts to create slam kernel.");

            AAsset* asset = AAssetManager_open(AssetManager::get(), "vocabulary/ORBVoc.txt", AASSET_MODE_BUFFER);
            assert(asset && "[Android Slam App Info] Failed to open ORBVoc.txt.");

            size_t size = AAsset_getLength(asset);
            auto buffer = (const char*) AAsset_getBuffer(asset);
            std::string voc_buffer(buffer, buffer + size);
            AAsset_close(asset);

            m_slam_kernel = std::make_unique<SlamKernel>(k_sensor_camera_width, k_sensor_camera_height, std::move(voc_buffer));

            DEBUG_INFO("[Android Slam App Info] Creates slam kernel successfully.");
        }


        // ImGui
        {
            IMGUI_CHECKVERSION();

            ImGui::CreateContext();
            ImGui::StyleColorsDark();

            ImFontConfig font_config{};
            font_config.SizePixels = 40.0f;

            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = nullptr;
            io.Fonts->AddFontDefault(&font_config);

            ImGui_ImplAndroid_Init(g_state->window);
            ImGui_ImplOpenGL3_Init("#version 300 es");
        }


        m_active = true;
    }

    void App::exit()
    {
        m_active = false;


        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplAndroid_Shutdown();
        ImGui::DestroyContext();


        m_image_pool.reset(nullptr);


        m_window.reset(nullptr);
    }

    void App::update(float dt)
    {
        // Slam handling.
        {
            static Timer slam_timer;
            std::vector<Image> images;
            images.push_back(Image{ m_image_pool->getImage() });

            TrackingResult tracking_res = m_slam_kernel->handleData(slam_timer.peek(), images, {});

            DEBUG_INFO("[Android Slam App Info] Current tracking state: %s", tracking_res.tracking_status.c_str());

            // Draw trajectory.
            glViewport(0, 0, m_window->getWidth(), m_window->getHeight());

            m_slam_renderer->setImage(k_sensor_camera_width, k_sensor_camera_height, images[0]);
            m_slam_renderer->setData(tracking_res);
            m_slam_renderer->draw();
        }


        // UI handling.
        {
            glViewport(0, 0, m_window->getWidth(), m_window->getHeight());

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplAndroid_NewFrame();
            ImGui::NewFrame();

            if(ImGui::CollapsingHeader("Android Slam"))
            {
                if(ImGui::TreeNode("Render Options"))
                {
                    if(ImGui::Button("Show / Hide Map Points"))
                    {
                        m_slam_renderer->m_show_mappoints = !m_slam_renderer->m_show_mappoints;
                    }

                    if(ImGui::Button("Show / Hide Key Frames"))
                    {
                        m_slam_renderer->m_show_keyframes = !m_slam_renderer->m_show_keyframes;
                    }

                    if(ImGui::Button("Show / Hide Image"))
                    {
                        m_slam_renderer->m_show_image = !m_slam_renderer->m_show_image;
                    }

                    ImGui::ColorEdit3("Map Point Color", reinterpret_cast<float*>(&m_slam_renderer->m_mp_color));
                    ImGui::ColorEdit3("Key Frame Color", reinterpret_cast<float*>(&m_slam_renderer->m_kf_color));
                    ImGui::ColorEdit3("Screen Clear Color", reinterpret_cast<float*>(&m_slam_renderer->m_clear_color));

                    ImGui::TreePop();
                }

                if(ImGui::TreeNode("App Options"))
                {
                    ImGui::Text("Current FPS %.2f.", 1.0f / dt);
                    if(ImGui::Button("Exit App"))
                    {
                        m_active = false;
                        m_running = false;
                    }

                    ImGui::TreePop();
                }
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }


        m_window->swapBuffers();

        DEBUG_INFO("[Android Slam App Info] Current FPS: %.2f frame per second.", 1.0f / dt);
    }

    void App::onCmd(android_app *app, int32_t cmd)
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

            if(new_width != instance.m_window->getWidth() || new_height != instance.m_window->getHeight())
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
        {}
        }
    }

    int32_t App::onInput(android_app* app, AInputEvent* ie)
    {
        (void)app;
        return ImGui_ImplAndroid_HandleInputEvent(ie);
    }

}
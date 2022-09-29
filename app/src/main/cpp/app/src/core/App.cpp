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


        m_active = true;
    }

    void App::exit()
    {
        m_active = false;


        m_image_pool.reset(nullptr);


        m_window.reset(nullptr);
    }

    void App::update(float dt)
    {
        // Clear buffers.
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Use GPU shader to trans YUV to RGB.
        //{
        //    std::vector<uint8_t> img = m_image_pool->getImage();
        //    Shader debug_shader("shader/yuv2rgb.vert", "shader/debug_texture.frag");
        //    Plane2D debug_plane;
        //    ImageTexture debug_texture(k_sensor_camera_width, k_sensor_camera_height, img);

        //    debug_plane.bind();
        //    debug_shader.bind();

        //    glActiveTexture(GL_TEXTURE0);
        //    debug_shader.setInt("screen_shot", 0);
        //    debug_texture.bind();

        //    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        //    debug_texture.unbind();
        //    debug_shader.unbind();
        //    debug_plane.unbind();
        //}


        // Slam handling.
        {
            static Timer slam_timer;
            std::vector<Image> images;
            images.push_back(Image{ m_image_pool->getImage() });
            TrackingResult tracking_res = m_slam_kernel->handleData(slam_timer.peek(), images, {});

            const auto& [last_pose, trajectory, map_points, tracking_state] = tracking_res;

            std::string state_str;
            switch (tracking_state)
            {
                case -1:
                    state_str = "SYSTEM_NOT_READY";
                    break;
                case 0:
                    state_str = "NO_IMAGES_YET";
                    break;
                case 1:
                    state_str = "NOT_INITIALIZED";
                    break;
                case 2:
                    state_str = "OK";
                    break;
                case 3:
                    state_str = "RECENTLY_LOST";
                    break;
                case 4:
                    state_str = "LOST";
                    break;
                case 5:
                    state_str = "OK_KLT";
                    break;
                default:
                    break;
            }

            DEBUG_INFO("[Android Slam App Info] Current tracking state: %s", state_str.c_str());


            // Draw trajectory.
            glViewport(0, 0, m_window->getWidth(), m_window->getHeight());

            // glm's ctor is col major.
            glm::mat4 mat_view(
                last_pose[+0], last_pose[+1], last_pose[+2], last_pose[+3],
                last_pose[+4], last_pose[+5], last_pose[+6], last_pose[+7],
                last_pose[+8], last_pose[+9], last_pose[10], last_pose[11],
                last_pose[12], last_pose[13], last_pose[14], last_pose[15]
            );
            static const glm::mat4 k_mat_change(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, -1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            );
            mat_view = k_mat_change * mat_view;

            glm::mat4 mat_proj = glm::perspective(glm::radians(45.0f), m_window->getAspectRatio(), 0.1f, 100.0f);


            uint32_t map_point_vao{};
            glGenVertexArrays(1, &map_point_vao);
            glBindVertexArray(map_point_vao);

            uint32_t map_point_vbo{};
            glGenBuffers(1, &map_point_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, map_point_vbo);
            glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(decltype(map_points)::value_type) * map_points.size()), map_points.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (const void*)0);
            glEnableVertexAttribArray(0);


            Shader mvp_shader("shader/mvp.vert", "shader/mvp.frag");
            mvp_shader.bind();
            mvp_shader.setMat4("u_mat_view", mat_view);
            mvp_shader.setMat4("u_mat_proj", mat_proj);
            mvp_shader.setVec3("u_color", glm::vec3{ 1.0f, 1.0f, 1.0f });


            glDrawArrays(GL_POINTS, 0, (GLsizei)map_points.size());
            DEBUG_INFO("[Android Slam App Info] Draw %ld points.", map_points.size());

            mvp_shader.unbind();
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            glDeleteBuffers(1, &map_point_vbo);
            glDeleteVertexArrays(1, &map_point_vao);
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
        App& instance = *static_cast<App*>(app->userData);

        int32_t event_type = AInputEvent_getType(ie);
        switch (event_type)
        {
        case AINPUT_EVENT_TYPE_KEY:
        case AINPUT_EVENT_TYPE_FOCUS:
        {
            break;
        }
        case AINPUT_EVENT_TYPE_MOTION:
        {
            int32_t action = AMotionEvent_getAction(ie);
            int32_t ptr_idx = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
            action &= AMOTION_EVENT_ACTION_MASK;

            int32_t tool_type = AMotionEvent_getToolType(ie, ptr_idx);

            const float x_value = AMotionEvent_getX(ie, ptr_idx);
            const float y_value = AMotionEvent_getY(ie, ptr_idx);

            switch (action)
            {
            case AMOTION_EVENT_ACTION_DOWN:
            {
                if (tool_type == AMOTION_EVENT_TOOL_TYPE_FINGER ||
                    tool_type == AMOTION_EVENT_TOOL_TYPE_UNKNOWN)
                {
                    instance.onMotionDown(x_value, y_value);
                }
                break;
            }
            case AMOTION_EVENT_ACTION_UP:
            {
                if (tool_type == AMOTION_EVENT_TOOL_TYPE_FINGER ||
                    tool_type == AMOTION_EVENT_TOOL_TYPE_UNKNOWN)
                {
                    instance.onMotionUp(x_value, y_value);
                }
                break;
            }
            case AMOTION_EVENT_ACTION_MOVE:
            {
                instance.onMotionMove(x_value, y_value);
                break;
            }
            case AMOTION_EVENT_ACTION_CANCEL:
            {
                instance.onMotionCancel(x_value, y_value);
                break;
            }
            default:
            {
                break;
            }
            }
        }
        default:
        {
            break;
        }
        }

        if(event_type == AINPUT_EVENT_TYPE_MOTION)
        {
            return 1;
        }
        return 0;
    }

    void App::onMotionDown(float x_pos, float y_pos)
    {
        DEBUG_INFO("Motion Down Event: [%.3f, %.3f]", x_pos, y_pos);
    }

    void App::onMotionUp(float x_pos, float y_pos)
    {
        DEBUG_INFO("Motion Up Event: [%.3f, %.3f]", x_pos, y_pos);
    }

    void App::onMotionMove(float x_pos, float y_pos)
    {
        DEBUG_INFO("Motion Move Event: [%.3f, %.3f]", x_pos, y_pos);
    }

    void App::onMotionCancel(float x_pos, float y_pos)
    {
        DEBUG_INFO("Motion Cancel Event: [%.3f, %.3f]", x_pos, y_pos);
    }

}
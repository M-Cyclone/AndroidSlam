#include "core/App.h"
#include <thread>
#include <chrono>
#include <atomic>

#include "utils/Log.h"
#include "utils/AssetManager.h"

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

        m_sensor_camera = std::make_unique<SensorCamera>(
                k_sensor_camera_width,
                k_sensor_camera_height,
                AIMAGE_FORMAT_YUV_420_888,
                AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT
        );
        m_sensor_camera->startCapture();


        // Camera image converter.
        {
            m_image_painter = std::make_unique<Plane2D>();

            m_sensor_texture = std::make_unique<SensorTexture>(
                k_sensor_camera_width,
                k_sensor_camera_height
            );

            m_yuv2rgb_shader = Shader::create(
                "shader/yuv2rgb.vert",
                "shader/yuv2rgb.frag"
            );
        }


        m_active = true;
    }

    void App::exit()
    {
        m_active = false;

        m_sensor_camera->stopCapture();
        m_window.reset(nullptr);
    }

    void App::update(float dt)
    {
        // Use GPU shader to trans YUV to RGB.
        {
            glViewport(0, 0, k_sensor_camera_width, k_sensor_camera_height);


            // Clear buffers.
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


            // Update Sensor Image.
            m_sensor_texture->setImage(m_sensor_camera->getLatestImage());


            // Do draw call.
            m_image_painter->bind();
            m_yuv2rgb_shader->bind();

            glActiveTexture(GL_TEXTURE0);
            m_sensor_texture->bind();
            m_yuv2rgb_shader->setInt("sensor_image", 0);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            m_sensor_texture->unbind();
            m_yuv2rgb_shader->unbind();
            m_image_painter->unbind();
        }


        // SLAM result present.
        {
            glViewport(0, 0, m_window->getWidth(), m_window->getHeight());

            // Clear buffers.
            //glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }


        m_window->swapBuffers();
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
#include "core/App.h"
#include <thread>
#include <chrono>
#include <atomic>

#include <opencv2/opencv.hpp>

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


        // SLAM
        {
            ORB_SLAM3::Settings::SettingDesc desc;
            desc.sensor = ORB_SLAM3::System::eSensor::MONOCULAR;
            desc.cameraInfo.cameraType = ORB_SLAM3::Settings::CameraType::PinHole;
            desc.cameraInfo.fx = 458.654f;
            desc.cameraInfo.fy = 457.296f;
            desc.cameraInfo.cx = 367.215f;
            desc.cameraInfo.cy = 248.375f;
            desc.distortion->k1 = -0.28340811f;
            desc.distortion->k2 = 0.07395907f;
            desc.distortion->p1 = 0.00019359f;
            desc.distortion->p2 = 1.76187114e-05f;
            desc.imageInfo.width = k_sensor_camera_width;
            desc.imageInfo.height = k_sensor_camera_height;
            desc.imageInfo.newWidth = 600;
            desc.imageInfo.newHeight = 350;
            desc.imageInfo.fps = 60;
            desc.imageInfo.bRGB = true;
            desc.imuInfo.noiseGyro = 1.7e-4f;
            desc.imuInfo.noiseAcc = 2.0000e-3f;
            desc.imuInfo.gyroWalk = 1.9393e-05f;
            desc.imuInfo.accWalk = 3.0000e-03f;
            desc.imuInfo.frequency = 200.0f;
            desc.imuInfo.cvTbc = static_cast<cv::Mat>(cv::Mat_<float>(4, 4) <<
                0.0148655429818f, -0.999880929698f, 0.00414029679422f, -0.0216401454975f,
                0.999557249008f, 0.0149672133247f, 0.025715529948f, -0.064676986768f,
                -0.0257744366974f, 0.00375618835797f, 0.999660727178f, 0.00981073058949f,
                0.0f, 0.0f, 0.0f, 1.0f
            );
            desc.imuInfo.bInsertKFsWhenLost = true;
            desc.orbInfo.nFeatures = 1000;
            desc.orbInfo.scaleFactor = 1.2f;
            desc.orbInfo.nLevels = 8;
            desc.orbInfo.initThFAST = 20;
            desc.orbInfo.minThFAST = 7;
            desc.viewerInfo.keyframeSize = 0.05f;
            desc.viewerInfo.keyframeLineWidth = 1.0f;
            desc.viewerInfo.graphLineWidth = 0.9f;
            desc.viewerInfo.pointSize = 2.0f;
            desc.viewerInfo.cameraSize = 0.08f;
            desc.viewerInfo.cameraLineWidth = 3.0f;
            desc.viewerInfo.viewPointX = 0.0f;
            desc.viewerInfo.viewPointY = -0.7f;
            desc.viewerInfo.viewPointZ = -3.5f;
            desc.viewerInfo.viewPointF = 500.0f;
            desc.viewerInfo.imageViewerScale = 1.0f;
            m_slam_settings = std::make_unique<ORB_SLAM3::Settings>(desc);


            auto asset = AAssetManager_open(AssetManager::get(), "vocabulary/ORBVoc.txt", AASSET_MODE_BUFFER);
            assert(asset && "[Android Slam App Info] Failed to open ORBVoc.txt.");

            size_t size = AAsset_getLength(asset);
            auto buffer = (const char*) AAsset_getBuffer(asset);
            std::string voc_buffer(buffer, buffer + size);
            AAsset_close(asset);

            m_slam_vocabulary = std::make_unique<ORB_SLAM3::ORBVocabulary>();
            DEBUG_INFO("[Android Slam App Info] Starts loading ORB Voc.");
            m_slam_vocabulary->loadFromAndroidTextFile(voc_buffer);
            DEBUG_INFO("[Android Slam App Info] Loading ORB Voc finished.");


            m_slam_kernel = std::make_unique<ORB_SLAM3::System>(
                m_slam_vocabulary.get(),
                m_slam_settings.get(),
                (const ORB_SLAM3::System::eSensor)(desc.sensor)
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

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void*)0);

            m_sensor_texture->unbind();
            m_yuv2rgb_shader->unbind();
            m_image_painter->unbind();
        }


        // SLAM calculation.
        {
            static Timer slam_timer;

            cv::Mat camera_image(k_sensor_camera_height, k_sensor_camera_width, CV_8UC4);
            glReadPixels(
                0,
                0,
                k_sensor_camera_width,
                k_sensor_camera_height,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                camera_image.data
            );

            cv::Mat gray_image;
            cv::cvtColor(camera_image, gray_image, cv::COLOR_RGBA2GRAY);

            m_slam_kernel->TrackMonocular(gray_image, slam_timer.peek());

            static size_t frame_count = 0;
            DEBUG_INFO("[Android Slam App Info] Current FPS: %.3f.", 1.0f / dt);
            DEBUG_INFO("[Android Slam App Info] Handled %zu frames.", frame_count);
            DEBUG_INFO("[Android Slam App Info] Current Map Point count: %zu.", m_slam_kernel->getAtlas().GetAllMapPoints().size());
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
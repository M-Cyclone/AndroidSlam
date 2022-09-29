#pragma once
#include <string>
#include <memory>

#include <android_native_app_glue.h>

#include <SlamKernel.h>

#include "Window.h"
#include "utils/Timer.h"
#include "sensor/SensorCamera.h"

#include "render/ImagePool.h"

namespace android_slam
{

    class App
    {
    private:
        static constexpr const char* k_app_name = "Android_SLAM";
        static constexpr const size_t k_max_fps = 20;
        static constexpr const float k_min_frame_time_second = 1.0f / (float)k_max_fps;

        static constexpr const int32_t k_sensor_camera_width = 640;
        static constexpr const int32_t k_sensor_camera_height = 480;

    public:
        App(android_app* state) noexcept;
        App(const App&) = delete;
        App& operator=(const App&) = delete;
        ~App() noexcept = default;

        void run();

    private:
        void init();
        void update(float dt);
        void exit();

        void onMotionDown(float x_pos, float y_pos);
        void onMotionUp(float x_pos, float y_pos);
        void onMotionMove(float x_pos, float y_pos);
        void onMotionCancel(float x_pos, float y_pos);

        static void onCmd(android_app* app, int32_t cmd);
        static int32_t onInput(android_app* app, AInputEvent* ie);

    private:
        Timer m_timer;
        std::unique_ptr<Window> m_window;
        std::unique_ptr<ImagePool> m_image_pool;
        std::unique_ptr<SlamKernel> m_slam_kernel;

    public:
        bool m_running = true;
        bool m_active = false;
    };

}
#pragma once
#include <string>
#include <memory>

#include <android_native_app_glue.h>

#include "Window.h"
#include "utils/Timer.h"
#include "sensor/SensorCamera.h"

#include "render/ImageTexture.h"
#include "render/SensorTexture.h"
#include "render/Shader.h"
#include "render/Plane2D.h"

namespace android_slam
{

    class App
    {
    private:
        static constexpr const char* k_app_name = "Android_SLAM";
        static constexpr const size_t k_max_fps = 60;
        static constexpr const float k_min_frame_time_second = 1.0f / (float)k_max_fps;

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
        std::unique_ptr<SensorCamera> m_sensor_camera;

        std::unique_ptr<Plane2D> m_image_painter;
        std::unique_ptr<SensorTexture> m_sensor_texture;
        std::shared_ptr<Shader> m_yuv2rgb_shader;

        std::unique_ptr<Plane2D> m_debug_painter;
        std::unique_ptr<ImageTexture> m_debug_texture;
        std::shared_ptr<Shader> m_debug_shader;

    public:
        bool m_running = true;
        bool m_active = false;
    };

}
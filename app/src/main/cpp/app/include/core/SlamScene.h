#pragma once
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>

#include <SlamKernel.h>

#include "core/Scene.h"

#include "sensor/SensorCamera.h"

#include "render/ImagePool.h"
#include "render/SlamRenderer.h"

#include "utils/Timer.h"

namespace android_slam
{

    class SlamScene : public Scene
    {
    private:
        static constexpr const int32_t k_sensor_camera_width = 640;
        static constexpr const int32_t k_sensor_camera_height = 480;
        static constexpr const float   k_fps_camera_fov = 45.0f;
        static constexpr const float   k_fps_camera_z_min = 0.1f;
        static constexpr const float   k_fps_camera_z_max = 1000.0f;

    public:
        explicit SlamScene(App& app, const char* name) noexcept
            : Scene(app, name)
        {}
        SlamScene(const SlamScene&) = delete;
        SlamScene& operator=(const SlamScene&) = delete;
        virtual ~SlamScene() noexcept = default;

        void init() override;
        void exit() override;

        void update(float dt) override;
        void drawGui(float dt) override;

    private:
        std::unique_ptr<ImagePool>    m_image_pool;
        std::unique_ptr<SlamRenderer> m_slam_renderer;
        std::unique_ptr<SlamKernel>   m_slam_kernel;

        std::unique_ptr<std::thread> m_slam_thread;
        std::vector<Image>           m_images;
        TrackingResult               m_tracking_result;
        std::mutex                   m_image_mutex;
        std::mutex                   m_tracking_res_mutex;
        std::atomic_bool             m_slam_has_new_image = false;
        std::atomic_bool             m_is_running_slam = true;

        bool m_need_update_image = true;
    };

}
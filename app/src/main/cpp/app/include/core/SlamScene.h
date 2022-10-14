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
        std::unique_ptr<ImagePool> m_image_pool;
        std::unique_ptr<SlamRenderer> m_slam_renderer;
        std::unique_ptr<SlamKernel> m_slam_kernel;
        Timer m_slam_timer;

        std::atomic_bool m_slam_has_new_image;
        std::mutex m_image_mutex;
        std::mutex m_tracking_res_mutex;
        std::vector<Image> m_images;
        std::unique_ptr<std::thread> m_slam_thread;
        TrackingResult m_tracking_result;
        std::atomic_bool m_is_running_slam = true;

        bool m_need_update_image = true;
    };

}
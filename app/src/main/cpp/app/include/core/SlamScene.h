#pragma once
#include <memory>

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
        explicit SlamScene(App& app) noexcept
            : Scene(app)
        {}
        SlamScene(const SlamScene&) = delete;
        SlamScene& operator=(const SlamScene&) = delete;
        virtual ~SlamScene() noexcept = default;

        void init() override;
        void exit() override;

        void update(float dt) override;
        void drawGui(float dt) override;

    private:


    private:
        std::unique_ptr<ImagePool> m_image_pool;
        std::unique_ptr<SlamRenderer> m_slam_renderer;
        std::unique_ptr<SlamKernel> m_slam_kernel;
        Timer m_slam_timer;

        bool m_is_running = true;
    };

}
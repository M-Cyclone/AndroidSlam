#pragma once
#include <memory>
#include <list>

#include "core/Scene.h"

#include "sensor/SensorCamera.h"

#include "render/ImagePool.h"
#include "render/Plane2D.h"
#include "render/Shader.h"
#include "render/ImageTexture.h"

#include "utils/Timer.h"

namespace android_slam
{

    class CalibScene : public Scene
    {
    private:
        static constexpr int32_t k_sensor_image_width = 640;
        static constexpr int32_t k_sensor_image_height = 480;

    public:
        explicit CalibScene(App& app, const char* name) noexcept
            : Scene(app, name)
            , m_image_shader("shader/image2d.vert", "shader/image2d.frag")
        {}
        CalibScene(const CalibScene&) = delete;
        CalibScene& operator=(const CalibScene&) = delete;
        virtual ~CalibScene() noexcept = default;

        void init() override;
        void exit() override;

        void update(float dt) override;
        void drawGui(float dt) override;

    private:
        std::unique_ptr<ImagePool> m_image_pool;
        Plane2D m_image_painter;
        Shader m_image_shader;

        std::list<std::pair<Image, std::unique_ptr<ImageTexture>>> m_calib_images;
        decltype(m_calib_images)::iterator m_last_delete_image;

        bool m_shot_img = false;
        bool m_calibrate = false;
    };

}
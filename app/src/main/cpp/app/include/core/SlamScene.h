#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

#include <SlamKernel.h>

#include "core/Scene.h"

#include "sensor/SensorCamera.h"
#include "sensor/SensorIMU.h"

#include "render/ImagePool.h"
#include "render/SlamRenderer.h"

#include "utils/Timer.h"

namespace android_slam
{

class SlamScene : public Scene
{
private:
    // 传感器相机图像尺寸
    static constexpr int32_t k_sensor_camera_width  = 640;
    static constexpr int32_t k_sensor_camera_height = 480;

    // 展示时渲染器虚拟相机的参数，详见glm::perspective()
    static constexpr float k_fps_camera_fov   = 45.0f;
    static constexpr float k_fps_camera_z_min = 0.1f;
    static constexpr float k_fps_camera_z_max = 1000.0f;

public:
    explicit SlamScene(App& app, const char* name) noexcept
        : Scene(app, name)
    {}
    SlamScene(const SlamScene&)            = delete;
    SlamScene& operator=(const SlamScene&) = delete;

    void init() override;
    void exit() override;

    void update(float dt) override;
    void drawGui(float dt) override;

private:
    std::unique_ptr<SensorIMU>    m_imu_pool;       // 用于获取IMU数据
    std::unique_ptr<ImagePool>    m_image_pool;     // 用于获取图像数据
    std::unique_ptr<SlamRenderer> m_slam_renderer;  // 渲染SLAM数据
    std::unique_ptr<SlamKernel>   m_slam_kernel;    // 真正执行SLAM算法的模块

    std::unique_ptr<std::thread> m_slam_thread;  // SLAM线程对象
    std::vector<Image>           m_images;       // 每帧获取的左右两张图像
    std::vector<ImuPoint>        m_imu_points;   // 每帧获取的IMU数据

    TrackingResult   m_tracking_result;            // 每帧SLAM解算结果
    std::mutex       m_image_mutex;                // 图像同步的互斥锁
    std::mutex       m_tracking_res_mutex;         // 结算结果的互斥锁
    std::atomic_bool m_slam_has_new_data = false;  // 带自旋锁的获取图像标志
    std::atomic_bool m_is_running_slam   = true;   // 带自旋锁的SLAM运行标志

    bool m_need_update_image = true;  // 用于标志是否在运行SLAM模块以提供新图像
};

}  // namespace android_slam
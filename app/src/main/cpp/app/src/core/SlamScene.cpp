#include "core/SlamScene.h"

#include "core/App.h"

#include "render/ImageTexture.h"
#include "render/Plane2D.h"
#include "render/Shader.h"


#include "utils/AssetManager.h"
#include "utils/Log.h"


namespace android_slam
{

void SlamScene::init()
{
    // 创建image pool，需绑定用于转换YUV到RGB的shader
    m_image_pool = std::make_unique<ImagePool>(k_sensor_camera_width,
                                               k_sensor_camera_height,
                                               "shader/yuv2rgb.vert",
                                               "shader/yuv2rgb.frag");
    // 刷新第一帧图像，用于保存初始时间戳
    Image first_image = m_image_pool->getImage();

    // 创建imu pool，用于获取IMU数据
    m_imu_pool = std::make_unique<SensorIMU>();

    // 创建slam renderer，根据设定参数创建，包括fov、长宽比、z轴范围
    m_slam_renderer = std::make_unique<SlamRenderer>(k_fps_camera_fov,
                                                     m_app_ref.getWindow().getAspectRatio(),
                                                     k_fps_camera_z_min,
                                                     k_fps_camera_z_max);

    // 创建SLAM对象
    {
        std::cout << "[Android Slam App Info] Starts to create slam kernel." << std::endl;

        // 读取ORB-SLAM3的词袋文件
        AAsset* asset = AAssetManager_open(AssetManager::get(), "vocabulary/ORBVoc.txt", AASSET_MODE_BUFFER);
        assert(asset && "[Android Slam App Info] Failed to open ORBVoc.txt.");

        size_t      size   = AAsset_getLength(asset);
        auto        buffer = (const char*)AAsset_getBuffer(asset);
        std::string voc_buffer(buffer, buffer + size);
        AAsset_close(asset);

        // 创建
        m_slam_kernel = std::make_unique<SlamKernel>(k_sensor_camera_width,
                                                     k_sensor_camera_height,
                                                     std::move(voc_buffer),
                                                     first_image.time_stamp);

        std::cout << "[Android Slam App Info] Creates slam kernel successfully." << std::endl;
    }

    // SLAM线程启动前默认为以下参数
    m_slam_has_new_data = false;  // 是否获取新图像到SLAM线程
    m_is_running_slam   = true;   // SLAM线程是否继续运行，为false时清理内存退出

    // 创建SLAM线程
    m_slam_thread = std::make_unique<std::thread>([this]()
    {
        // 暂时缓存图像和imu数据
        std::vector<Image>    images;
        std::vector<ImuPoint> imu_points;

        // SLAM线程主循环
        while (m_is_running_slam)
        {
            // 如果需要进行SLAM跟踪
            if (m_slam_has_new_data)
            {
                // 更新图像和IMU数据
                {
                    std::unique_lock<std::mutex> lock(m_image_mutex);
                    images     = std::move(m_images);
                    imu_points = std::move(m_imu_points);
                }

                // SLAM解算
                auto res            = m_slam_kernel->handleData(images, imu_points);
                m_slam_has_new_data = false;  // This image is processed and this thread needs new image.

                // 与主线程同步SLAM结算结果
                {
                    std::unique_lock<std::mutex> lock(m_tracking_res_mutex);
                    m_tracking_result = std::move(res);
                }
            }
        }
    });
}

void SlamScene::exit()
{
    m_is_running_slam   = false;
    m_need_update_image = false;
    m_slam_thread->join();  // 退出时线程join，之后再清理内存
    m_slam_thread.reset(nullptr);

    m_slam_kernel.reset(nullptr);
    m_slam_renderer.reset(nullptr);
    m_image_pool.reset(nullptr);
}

void SlamScene::update(float dt)
{
    // 如果征程工作且需要更新图像
    if (m_need_update_image && !m_slam_has_new_data)
    {
        // Slam handling.
        std::vector<Image> images;
        {
            // 从Image Pool取出图像，转换到RGB后传入SLAM线程
            images.push_back(m_image_pool->getImage());

            std::unique_lock<std::mutex> lock(m_image_mutex);
            m_images     = images;
            m_imu_points = m_imu_pool->getImuData();
        }
        m_slam_has_new_data = true;

        // 同步上一帧计算结果
        TrackingResult tracking_res;
        {
            std::unique_lock<std::mutex> lock(m_tracking_res_mutex);
            tracking_res = std::move(m_tracking_result);
        }

        // 设置SLAM渲染器数据
        m_slam_renderer->setImage(k_sensor_camera_width, k_sensor_camera_height, images[0]);
        m_slam_renderer->setData(tracking_res);

        m_app_ref.m_last_process_delta_time = tracking_res.processing_delta_time;
    }

    // 渲染
    m_slam_renderer->clearColor();  // 清屏

    int32_t screen_width  = m_app_ref.getWindow().getWidth();
    int32_t screen_height = m_app_ref.getWindow().getHeight();

    m_slam_renderer->drawMapPoints(0, 0, screen_width, screen_height);
    m_slam_renderer->drawKeyFrames(0, 0, screen_width, screen_height);

    // 这里是希望保证俯视图轨迹为1:1而图像展示为4:3，因此手动计算的参数
    int32_t img_width  = screen_height * 4 / 7;
    int32_t img_height = screen_height * 3 / 7;
    m_slam_renderer->drawImage(0, 0, img_width, img_height);  // Aspect ratio: 4 : 3

    m_slam_renderer->drawTotalTrajectory(0, img_height, img_width, screen_height - img_height);  // Aspect ratio: 1 : 1
}

void SlamScene::drawGui(float dt)
{
    if (ImGui::TreeNode(u8"SLAM控制"))
    {
        // auto pos = m_slam_renderer->temp_last_kf_position;
        // ImGui::Text("Position: (%f, %f, %f)", pos.x, pos.y, pos.z);

        if (ImGui::Button(m_need_update_image ? u8"暂停" : u8"继续"))
        {
            m_need_update_image = !m_need_update_image;
        }

        // if (ImGui::Button(u8"重置"))
        //{
        //     m_slam_kernel->reset();
        // }

        if (ImGui::Button(u8"退出"))
        {
            m_app_ref.setActiveScene("Init");
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode(u8"绘制选项"))
    {
        if (ImGui::TreeNode(u8"地图点"))
        {
            if (ImGui::Button(m_slam_renderer->m_show_mappoints ? u8"隐藏" : u8"显示"))
            {
                m_slam_renderer->m_show_mappoints = !m_slam_renderer->m_show_mappoints;
            }

            ImGui::SliderFloat(u8"地图点大小", &m_slam_renderer->m_point_size, 1.0f, 5.0f);

            ImGui::ColorEdit3(u8"地图点颜色", reinterpret_cast<float*>(&m_slam_renderer->m_mp_color));

            ImGui::TreePop();
        }

        if (ImGui::TreeNode(u8"相机轨迹"))
        {
            if (ImGui::Button(m_slam_renderer->m_show_keyframes ? u8"隐藏" : u8"显示"))
            {
                m_slam_renderer->m_show_keyframes = !m_slam_renderer->m_show_keyframes;
            }

            ImGui::SliderFloat(u8"轨迹线粗细", &m_slam_renderer->m_line_width, 1.0f, 5.0f);

            ImGui::ColorEdit3(u8"轨迹线颜色", reinterpret_cast<float*>(&m_slam_renderer->m_kf_color));

            ImGui::TreePop();
        }

        if (ImGui::TreeNode(u8"其他"))
        {
            if (ImGui::Button(m_slam_renderer->m_show_image ? u8"隐藏相机图像" : u8"显示相机图像"))
            {
                m_slam_renderer->m_show_image = !m_slam_renderer->m_show_image;
            }

            if (ImGui::Button(m_slam_renderer->m_show_total_trajectory ? u8"隐藏全局轨迹" : u8"显示全局轨迹"))
            {
                m_slam_renderer->m_show_total_trajectory = !m_slam_renderer->m_show_total_trajectory;
            }

            ImGui::ColorEdit3(u8"背景颜色", reinterpret_cast<float*>(&m_slam_renderer->m_clear_color));

            ImGui::TreePop();
        }

        ImGui::TreePop();
    }
}

}  // namespace android_slam
#include "core/SlamScene.h"

#include "core/App.h"

#include "render/ImageTexture.h"
#include "render/Shader.h"
#include "render/Plane2D.h"

#include "utils/Log.h"
#include "utils/AssetManager.h"

namespace android_slam
{

    void SlamScene::init()
    {
        // Camera image converter.
        m_image_pool = std::make_unique<ImagePool>(
            k_sensor_camera_width,
            k_sensor_camera_height,
            "shader/yuv2rgb.vert",
            "shader/yuv2rgb.frag"
        );

        m_slam_renderer = std::make_unique<SlamRenderer>(45.0f, m_app.getWindow().getAspectRatio(), 0.1f, 1000.0f);

        {
            DEBUG_INFO("[Android Slam App Info] Starts to create slam kernel.");

            AAsset* asset = AAssetManager_open(AssetManager::get(), "vocabulary/ORBVoc.txt", AASSET_MODE_BUFFER);
            assert(asset && "[Android Slam App Info] Failed to open ORBVoc.txt.");

            size_t size = AAsset_getLength(asset);
            auto buffer = (const char*) AAsset_getBuffer(asset);
            std::string voc_buffer(buffer, buffer + size);
            AAsset_close(asset);

            m_slam_kernel = std::make_unique<SlamKernel>(k_sensor_camera_width, k_sensor_camera_height, std::move(voc_buffer));

            DEBUG_INFO("[Android Slam App Info] Creates slam kernel successfully.");
        }

        (void)m_slam_timer.mark();
    }

    void SlamScene::exit()
    {
        m_slam_kernel.reset(nullptr);
        m_slam_renderer.reset(nullptr);
        m_image_pool.reset(nullptr);
    }

    void SlamScene::update(float dt)
    {
        if (m_is_running_slam)
        {
            // Slam handling.
            std::vector<Image> images;
            images.push_back(Image{ m_image_pool->getImage() });

            TrackingResult tracking_res = m_slam_kernel->handleData(m_slam_timer.peek(), images, {});

            DEBUG_INFO("[Android Slam App Info] Current tracking state: %s", tracking_res.tracking_status.c_str());

            // Draw trajectory.
            glViewport(0, 0, m_app.getWindow().getWidth(), m_app.getWindow().getHeight());

            m_slam_renderer->setImage(k_sensor_camera_width, k_sensor_camera_height, images[0]);
            m_slam_renderer->setData(tracking_res);
        }

        m_slam_renderer->draw();
    }

    void SlamScene::drawGui(float dt)
    {
        if(ImGui::TreeNode(u8"SLAM选项"))
        {
            if(ImGui::TreeNode(u8"SLAM控制"))
            {
                if(ImGui::Button(m_is_running_slam ? u8"暂停" : u8"继续"))
                {
                    m_is_running_slam = !m_is_running_slam;
                }

                if(ImGui::Button(u8"重置"))
                {
                    m_slam_kernel->reset();
                }

                if(ImGui::Button(u8"退出"))
                {
                    m_app.setActiveScene("Init");
                }

                ImGui::TreePop();
            }

            if(ImGui::TreeNode(u8"绘制选项"))
            {
                if(ImGui::TreeNode(u8"地图点"))
                {
                    if(ImGui::Button(m_slam_renderer->m_show_mappoints ? u8"隐藏" : u8"显示"))
                    {
                        m_slam_renderer->m_show_mappoints = !m_slam_renderer->m_show_mappoints;
                    }

                    ImGui::SliderFloat(u8"地图点大小", &m_slam_renderer->m_point_size, 1.0f, 5.0f);

                    ImGui::ColorEdit3(u8"地图点颜色", reinterpret_cast<float*>(&m_slam_renderer->m_mp_color));

                    ImGui::TreePop();
                }

                if(ImGui::TreeNode(u8"相机轨迹"))
                {
                    if(ImGui::Button(m_slam_renderer->m_show_keyframes ? u8"隐藏" : u8"显示"))
                    {
                        m_slam_renderer->m_show_keyframes = !m_slam_renderer->m_show_keyframes;
                    }

                    ImGui::SliderFloat(u8"轨迹线粗细", &m_slam_renderer->m_line_width, 1.0f, 5.0f);

                    ImGui::ColorEdit3(u8"轨迹线颜色", reinterpret_cast<float*>(&m_slam_renderer->m_kf_color));

                    ImGui::TreePop();
                }

                if(ImGui::TreeNode(u8"相机图像"))
                {
                    if(ImGui::Button(m_slam_renderer->m_show_image ? u8"隐藏" : u8"显示"))
                    {
                        m_slam_renderer->m_show_image = !m_slam_renderer->m_show_image;
                    }

                    ImGui::TreePop();
                }

                if(ImGui::TreeNode(u8"其他"))
                {
                    ImGui::ColorEdit3(u8"背景颜色", reinterpret_cast<float*>(&m_slam_renderer->m_clear_color));

                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
    }

}
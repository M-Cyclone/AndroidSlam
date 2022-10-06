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
        if(ImGui::TreeNode("Slam Options"))
        {
            if(ImGui::TreeNode("Runtime Operation"))
            {
                if(ImGui::Button(m_is_running_slam ? "Pause" : "Resume"))
                {
                    m_is_running_slam = !m_is_running_slam;
                }

                if(ImGui::Button("Reset"))
                {
                    m_slam_kernel->reset();
                }

                if(ImGui::Button("Exit"))
                {
                    m_app.setActiveScene("Init");
                }

                ImGui::TreePop();
            }

            if(ImGui::TreeNode("Render Options"))
            {
                if(ImGui::TreeNode("Map Points"))
                {
                    if(ImGui::Button(m_slam_renderer->m_show_mappoints ? "Hide" : "Show"))
                    {
                        m_slam_renderer->m_show_mappoints = !m_slam_renderer->m_show_mappoints;
                    }

                    ImGui::SliderFloat("Point Size", &m_slam_renderer->m_point_size, 1.0f, 5.0f);

                    ImGui::ColorEdit3("Map Point Color", reinterpret_cast<float*>(&m_slam_renderer->m_mp_color));

                    ImGui::TreePop();
                }

                if(ImGui::TreeNode("Key Frames"))
                {
                    if(ImGui::Button(m_slam_renderer->m_show_keyframes ? "Hide" : "Show"))
                    {
                        m_slam_renderer->m_show_keyframes = !m_slam_renderer->m_show_keyframes;
                    }

                    ImGui::SliderFloat("Line Width", &m_slam_renderer->m_line_width, 1.0f, 5.0f);

                    ImGui::ColorEdit3("Key Frame Color", reinterpret_cast<float*>(&m_slam_renderer->m_kf_color));

                    ImGui::TreePop();
                }

                if(ImGui::TreeNode("Image Preview"))
                {
                    if(ImGui::Button(m_slam_renderer->m_show_image ? "Hide" : "Show"))
                    {
                        m_slam_renderer->m_show_image = !m_slam_renderer->m_show_image;
                    }

                    ImGui::TreePop();
                }

                if(ImGui::TreeNode("Others"))
                {
                    ImGui::ColorEdit3("Screen Clear Color", reinterpret_cast<float*>(&m_slam_renderer->m_clear_color));

                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
    }

}
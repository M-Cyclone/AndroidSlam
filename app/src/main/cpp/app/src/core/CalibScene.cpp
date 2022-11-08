#include "core/CalibScene.h"
#include <vector>

#include <opencv2/opencv.hpp>

#include "core/App.h"

#include "utils/Log.h"

namespace android_slam
{

    void CalibScene::init()
    {
        m_image_pool = std::make_unique<ImagePool>(k_sensor_image_width,
                                                   k_sensor_image_height,
                                                   "shader/yuv2rgb.vert",
                                                   "shader/yuv2rgb.frag");

        m_last_delete_image = m_calib_images.end();
    }

    void CalibScene::exit()
    {
        m_image_pool.reset(nullptr);
    }

    void CalibScene::update(float dt)
    {
        if(!m_calibrate)
        {
            if (m_last_delete_image != m_calib_images.end())
            {
                m_calib_images.erase(m_last_delete_image);
            }
            m_last_delete_image = m_calib_images.end();


            Image img = m_image_pool->getImage();
            std::unique_ptr<ImageTexture> image_texture = std::make_unique<ImageTexture>(
                k_sensor_image_width,
                k_sensor_image_height,
                img.data
                );

            glViewport(0, 0, k_sensor_image_width * 2, k_sensor_image_height * 2);
            {
                m_image_painter.bind();
                m_image_shader.bind();

                glActiveTexture(GL_TEXTURE0);
                m_image_shader.setInt("screen_shot", 0);
                image_texture->bind();

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                image_texture->unbind();
                m_image_shader.unbind();
                m_image_painter.unbind();
            }

            if (m_shot_img)
            {
                m_calib_images.emplace_back(std::move(img), std::move(image_texture));
                m_shot_img = false;
            }
        }
        else
        {
            try
            {
                std::vector<cv::Mat> cv_images;
                std::vector<std::vector<cv::Point2f>> image_corners;

                cv::Size chessboard_size(1, 1);

                for (const auto&[img, tex]: m_calib_images)
                {
                    cv::Mat cv_image(k_sensor_image_height, k_sensor_image_width, CV_8UC3);
                    memcpy(cv_image.data, img.data.data(), sizeof(uint8_t) * img.data.size());
                    cv_images.push_back(cv_image);


                    std::vector<cv::Point2f> corners;
                    if (!cv::findChessboardCorners(
                    cv_image, chessboard_size, corners, cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE + cv::CALIB_CB_FAST_CHECK
                    ))
                    {
                        continue;
                    }

                    if (!cv::find4QuadCornerSubpix(cv_image, corners, { 5, 5 }))
                    {
                        continue;
                    }

                    image_corners.push_back(std::move(corners));
                }


                cv::Size chessboard_square_size(10, 10);
                std::vector<std::vector<cv::Point3f>> object_3d_points;

                cv::Mat camera_K_mat(3, 3, CV_32FC1, cv::Scalar::all(0));
                cv::Mat camera_dist_coeffs(1, 5, CV_32FC1, cv::Scalar::all(0));
                std::vector<size_t> corner_count_in_images;
                std::vector<cv::Mat> images_translation;
                std::vector<cv::Mat> images_rotation;

                size_t image_count = cv_images.size();
                for (size_t i = 0; i < image_count; ++i)
                {
                    std::vector<cv::Point3f> points;

                    for (int y = 0; y < chessboard_size.height; ++y)
                    {
                        for (int x = 0; x < chessboard_size.width; ++x)
                        {
                            points.emplace_back(
                            y * chessboard_square_size.width, x * chessboard_square_size.height, 0
                            );
                        }
                    }

                    corner_count_in_images.push_back(points.size());
                    object_3d_points.push_back(std::move(points));
                }


                double rms = cv::calibrateCamera(
                object_3d_points, image_corners, { k_sensor_image_width, k_sensor_image_height }, camera_K_mat, camera_dist_coeffs, images_rotation, images_translation, 0
                );

                std::ostringstream oss;
                oss << "[Calibrate Result]\n" << "[Camera K]\n" << camera_K_mat << "\n" << "[Camera Dist Coeffs]\n" << camera_dist_coeffs << "\n" << "[RMSE]\n" << rms << std::endl;
                std::string str = oss.str();

                DEBUG_INFO("%s", str.c_str());
            }
            catch (...)
            {
                DEBUG_INFO("OpenCV meets errors.");
            }
        }
    }

    void CalibScene::drawGui(float dt)
    {
        if(ImGui::TreeNode(u8"控制"))
        {
            if (ImGui::Button(u8"退出"))
            {
                m_app.setActiveScene("Init");
            }

            if(ImGui::Button(u8"标定"))
            {
                m_calibrate = true;
            }

            ImGui::TreePop();
        }

        if(ImGui::TreeNode("Image Info"))
        {
            if(ImGui::Button(u8"拍摄"))
            {
                m_shot_img = true;
            }
            ImGui::Text("Current image count: %d", (int)m_calib_images.size());
            int i = 0;
            for(auto it = m_calib_images.begin(); it != m_calib_images.end(); ++it)
            {
                ImGui::Image((void*)it->second->getId(), {160, 120}, {0, 1}, {1, 0});

                ImGui::SameLine();
                std::string str(u8"删除");
                str += std::to_string(i++);
                if(ImGui::Button(str.c_str()))
                {
                    m_last_delete_image = it;
                }
            }

            ImGui::TreePop();
        }
    }

}
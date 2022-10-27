#include "SlamKernel.h"
#include <iostream>
#include <set>
#include <unordered_set>

#include <opencv2/opencv.hpp>
#include <sophus/se3.hpp>

#include <core/System.h>
#include <utils/Settings.h>
#include <feature/ORBVocabulary.h>

namespace android_slam
{

    SlamKernel::SlamKernel(int32_t img_width, int32_t img_height, std::string vocabulary_data, int64_t begin_time_stamp)
    : m_width(img_width)
    , m_height(img_height)
    , m_begin_time_stamp(begin_time_stamp)
    , m_last_time(std::chrono::steady_clock::now())
    {
        ORB_SLAM3::Settings::SettingDesc desc{};
        desc.sensor = ORB_SLAM3::System::eSensor::IMU_MONOCULAR;
        desc.cameraInfo.cameraType = ORB_SLAM3::Settings::CameraType::PinHole;
        desc.cameraInfo.fx = 458.654f;
        desc.cameraInfo.fy = 457.296f;
        desc.cameraInfo.cx = 367.215f;
        desc.cameraInfo.cy = 248.375f;
        desc.distortion->k1 = -0.28340811f;
        desc.distortion->k2 = 0.07395907f;
        desc.distortion->p1 = 0.00019359f;
        desc.distortion->p2 = 1.76187114e-05f;
        desc.imageInfo.width = img_width;
        desc.imageInfo.height = img_height;
        desc.imageInfo.newWidth = 640;
        desc.imageInfo.newHeight = 480;
        desc.imageInfo.fps = 20;
        desc.imageInfo.bRGB = true;
        desc.imuInfo.noiseGyro = 1.7e-4f;
        desc.imuInfo.noiseAcc = 2.0000e-3f;
        desc.imuInfo.gyroWalk = 1.9393e-05f;
        desc.imuInfo.accWalk = 3.0000e-03f;
        desc.imuInfo.frequency = 200.0f;
        desc.imuInfo.cvTbc = static_cast<cv::Mat>(cv::Mat_<float>(4, 4) << +0.0148655429818f, -0.99988092969800f, +0.00414029679422f, -0.02164014549750f, +0.9995572490080f, +0.01496721332470f, +0.02571552994800f, -0.06467698676800f, -0.0257744366974f, +0.00375618835797f, +0.99966072717800f, +0.00981073058949f, 0.0f, 0.0f, 0.0f, 1.0f
        );
        desc.imuInfo.bInsertKFsWhenLost = true;
        desc.orbInfo.nFeatures = 1000;
        desc.orbInfo.scaleFactor = 1.2f;
        desc.orbInfo.nLevels = 8;
        desc.orbInfo.initThFAST = 20;
        desc.orbInfo.minThFAST = 7;
        desc.viewerInfo.keyframeSize = 0.05f;
        desc.viewerInfo.keyframeLineWidth = 1.0f;
        desc.viewerInfo.graphLineWidth = 0.9f;
        desc.viewerInfo.pointSize = 2.0f;
        desc.viewerInfo.cameraSize = 0.08f;
        desc.viewerInfo.cameraLineWidth = 3.0f;
        desc.viewerInfo.viewPointX = 0.0f;
        desc.viewerInfo.viewPointY = -0.7f;
        desc.viewerInfo.viewPointZ = -3.5f;
        desc.viewerInfo.viewPointF = 500.0f;
        desc.viewerInfo.imageViewerScale = 1.0f;
        auto slam_settings = new ::ORB_SLAM3::Settings(desc);


        auto vocabulary = new ::ORB_SLAM3::ORBVocabulary();
        vocabulary->loadFromAndroidTextFile(std::move(vocabulary_data));


        m_orb_slam = std::make_unique<::ORB_SLAM3::System>(
        vocabulary, slam_settings, static_cast<const ORB_SLAM3::System::eSensor>(desc.sensor));
    }

    // unique_ptr needs to know how to delete the ptr, so the dtor should be impl with the definition of the ptr class.
    SlamKernel::~SlamKernel()
    {
        m_orb_slam->Shutdown();
    }

    void SlamKernel::reset()
    {
        m_orb_slam->Reset();
    }

    TrackingResult SlamKernel::handleData(const std::vector<Image>& images, const std::vector<ImuPoint>& imus)
    {
        // Image assertion.
        const Image& image = images[0];
        assert((image.time_stamp >= m_begin_time_stamp) && "Invalid time stamp.");


        // Create OpenCV image.
        cv::Mat cv_image(m_height, m_width, CV_8UC3);
        memcpy(cv_image.data, image.data.data(), sizeof(uint8_t) * image.data.size());


        // Convert imu data.
        std::vector<ORB_SLAM3::IMU::Point> orb_imus;
        orb_imus.reserve(imus.size());
        for(auto [ax, ay, az, wx, wy, wz, ts] : imus)
        {
            orb_imus.emplace_back(-ax, ay, az, -wx, wy, wz, (double)(ts - m_begin_time_stamp) * k_nano_sec_to_sec_radio);
        }


        // Get time stamp from android time stamp.
        double image_time_stamp = (double)(image.time_stamp - m_begin_time_stamp) * k_nano_sec_to_sec_radio;
        // Tracking.
        Sophus::SE3f pose = m_orb_slam->TrackMonocular(cv_image, image_time_stamp, orb_imus);


        // Set tracking result.
        TrackingResult res;

        // Pose.
        {
            Eigen::Matrix4f mat_pose = pose.matrix();

            res.last_pose[+0] = mat_pose(0, 0);
            res.last_pose[+1] = mat_pose(1, 0);
            res.last_pose[+2] = mat_pose(2, 0);
            res.last_pose[+3] = mat_pose(3, 0);
            res.last_pose[+4] = mat_pose(0, 1);
            res.last_pose[+5] = mat_pose(1, 1);
            res.last_pose[+6] = mat_pose(2, 1);
            res.last_pose[+7] = mat_pose(3, 1);
            res.last_pose[+8] = mat_pose(0, 2);
            res.last_pose[+9] = mat_pose(1, 2);
            res.last_pose[10] = mat_pose(2, 2);
            res.last_pose[11] = mat_pose(3, 2);
            res.last_pose[12] = mat_pose(0, 3);
            res.last_pose[13] = mat_pose(1, 3);
            res.last_pose[14] = mat_pose(2, 3);
            res.last_pose[15] = mat_pose(3, 3);
        }


        // Key frames and map points.
        if (ORB_SLAM3::Map* active_map = m_orb_slam->getAtlas().GetCurrentMap())
        {
            {
                std::vector<ORB_SLAM3::KeyFrame*> key_frames = active_map->GetAllKeyFrames();

                static auto key_frame_cmp = [](const ORB_SLAM3::KeyFrame* kf1, const ORB_SLAM3::KeyFrame* kf2) -> bool
                {
                    if (!kf1 || !kf2) return true;

                    return kf1->mnFrameId < kf2->mnFrameId;
                };
                std::set<ORB_SLAM3::KeyFrame*, decltype(key_frame_cmp)> kf_set(key_frame_cmp);
                for (ORB_SLAM3::KeyFrame* kf: key_frames)
                {
                    if (!kf || kf->isBad()) continue;

                    kf_set.insert(kf);
                }

                for (ORB_SLAM3::KeyFrame* kf: kf_set)
                {
                    if (!kf || kf->isBad()) continue;

                    Eigen::Vector3f position = kf->GetPoseInverse().translation();
                    res.trajectory.push_back({ position.x(), position.y(), position.z() });
                }

                Eigen::Vector3f last_position = pose.inverse().translation();
                res.trajectory.push_back({ last_position.x(), last_position.y(), last_position.z() });
            }

            {
                std::vector<ORB_SLAM3::MapPoint*> local_mps = active_map->GetReferenceMapPoints();
                std::unordered_set<ORB_SLAM3::MapPoint*> local_mp_ust(local_mps.begin(), local_mps.end());

                for (ORB_SLAM3::MapPoint* mp: local_mps)
                {
                    if (!mp || mp->isBad()) continue;

                    Eigen::Vector3f position = mp->GetWorldPos();
                    res.map_points.push_back({ position.x(), position.y(), position.z() });
                }

                std::vector<ORB_SLAM3::MapPoint*> all_mps = active_map->GetAllMapPoints();
                for (ORB_SLAM3::MapPoint* mp: all_mps)
                {
                    if (!mp || mp->isBad() || local_mp_ust.find(mp) != local_mp_ust.end())
                        continue;

                    Eigen::Vector3f position = mp->GetWorldPos();
                    res.map_points.push_back({ position.x(), position.y(), position.z() });
                }
            }
        }


        // Tracking status string.
        {
            int status = m_orb_slam->getTrackingState();
            switch (status)
            {
                case -1:
                    res.tracking_status = "SYSTEM_NOT_READY";
                    break;
                case 0:
                    res.tracking_status = "NO_IMAGES_YET";
                    break;
                case 1:
                    res.tracking_status = "NOT_INITIALIZED";
                    break;
                case 2:
                    res.tracking_status = "OK";
                    break;
                case 3:
                    res.tracking_status = "RECENTLY_LOST";
                    break;
                case 4:
                    res.tracking_status = "LOST";
                    break;
                case 5:
                    res.tracking_status = "OK_KLT";
                    break;
                default:
                    break;
            }
        }


        // Processing time cost.
        {
            const std::chrono::steady_clock::time_point curr_time = m_last_time;
            m_last_time = std::chrono::steady_clock::now();

            res.processing_delta_time = std::chrono::duration<float>(m_last_time - curr_time).count();
        }

        return res;
    }

}
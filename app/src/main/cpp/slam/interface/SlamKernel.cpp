#include "SlamKernel.h"
#include <iostream>

#include <opencv2/opencv.hpp>
#include <sophus/se3.hpp>

#include <core/System.h>
#include <utils/Settings.h>
#include <feature/ORBVocabulary.h>

namespace android_slam
{

    SlamKernel::SlamKernel(int32_t img_width, int32_t img_height, std::string vocabulary_data)
        : m_width(img_width)
        , m_height(img_height)
    {
        ORB_SLAM3::Settings::SettingDesc desc{};
        desc.sensor = ORB_SLAM3::System::eSensor::MONOCULAR;
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
        desc.imageInfo.fps = 60;
        desc.imageInfo.bRGB = true;
        desc.imuInfo.noiseGyro = 1.7e-4f;
        desc.imuInfo.noiseAcc = 2.0000e-3f;
        desc.imuInfo.gyroWalk = 1.9393e-05f;
        desc.imuInfo.accWalk = 3.0000e-03f;
        desc.imuInfo.frequency = 200.0f;
        desc.imuInfo.cvTbc = static_cast<cv::Mat>(cv::Mat_<float>(4, 4)
        <<  0.0148655429818f, -0.999880929698f, 0.00414029679422f, -0.0216401454975f,
            0.999557249008f, 0.0149672133247f, 0.025715529948f, -0.064676986768f,
            -0.0257744366974f, 0.00375618835797f, 0.999660727178f, 0.00981073058949f,
            0.0f, 0.0f, 0.0f, 1.0f
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
            vocabulary,
            slam_settings,
            (const ORB_SLAM3::System::eSensor)(desc.sensor)
        );
    }

    // unique_ptr needs to know how to delete the ptr, so the dtor should be impl with the definition of the ptr class.
    SlamKernel::~SlamKernel() = default;

    std::tuple<size_t, size_t, int> SlamKernel::handleData(float time, const std::vector<Image>& images, const std::vector<ImuPoint>& imus)
    {
        cv::Mat cv_image(m_height, m_width, CV_8UC3);
        memcpy(cv_image.data, images[0].data.data(), sizeof(uint8_t) * images[0].data.size());
        Sophus::SE3f pose = m_orb_slam->TrackMonocular(cv_image, time);

        return
        {
            m_orb_slam->getAtlas().GetAllKeyFrames().size(),
            m_orb_slam->getAtlas().GetAllMapPoints().size(),
            m_orb_slam->getTrackingState()
        };
    }

}
#pragma once
#include <string>
#include <memory>
#include <vector>

namespace ORB_SLAM3
{
    class System;
}

namespace android_slam
{

    struct Image
    {
        std::vector<uint8_t> data;
    };

    struct ImuPoint
    {
        float ax;
        float ay;
        float az;
        float wx;
        float wy;
        float wz;
    };

    class SlamKernel
    {
    public:
        SlamKernel(int32_t img_width, int32_t img_height, std::string vocabulary_data);
        SlamKernel(const SlamKernel&) = delete;
        SlamKernel& operator=(const SlamKernel&) = delete;
        ~SlamKernel();

        void handleData(float time, const std::vector<Image>& images, const std::vector<ImuPoint>& imus);

    private:
        int32_t m_width;
        int32_t m_height;
        std::unique_ptr<::ORB_SLAM3::System> m_orb_slam;
    };

}
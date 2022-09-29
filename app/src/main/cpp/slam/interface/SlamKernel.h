#pragma once
#include <string>
#include <memory>
#include <vector>
#include <tuple>
#include <array>

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

    struct TrackingResult
    {
        struct Pos
        {
            float x;
            float y;
            float z;
        };

        std::array<float, 16> last_pose;
        std::vector<Pos> trajectory;
        std::vector<Pos> map_points;
        int tracking_status;
    };

    class SlamKernel
    {
    public:
        SlamKernel(int32_t img_width, int32_t img_height, std::string vocabulary_data);
        SlamKernel(const SlamKernel&) = delete;
        SlamKernel& operator=(const SlamKernel&) = delete;
        ~SlamKernel();

        TrackingResult handleData(float time, const std::vector<Image>& images, const std::vector<ImuPoint>& imus);

    private:
        int32_t m_width;
        int32_t m_height;
        std::unique_ptr<::ORB_SLAM3::System> m_orb_slam;
    };

}
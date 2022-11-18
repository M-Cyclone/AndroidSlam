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
        int64_t time_stamp;

        Image() noexcept = default;
        Image(std::vector<uint8_t> data, int64_t time_stamp) noexcept
            : data(std::move(data))
            , time_stamp(time_stamp)
        {}
    };

    struct ImuPoint
    {
        float ax;
        float ay;
        float az;
        float wx;
        float wy;
        float wz;
        int64_t time_stamp;

        ImuPoint() noexcept = default;
        ImuPoint(float ax, float ay, float az, float wx, float wy, float wz, int64_t time_stamp) noexcept
            : ax(ax)
            , ay(ay)
            , az(az)
            , wx(wx)
            , wy(wy)
            , wz(wz)
            , time_stamp(time_stamp)
        {}
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

        float processing_delta_time;
    };

    class SlamKernel
    {
    private:
        static constexpr int64_t k_nano_second_in_one_second = 1000000000;
        static constexpr double k_nano_sec_to_sec_radio = 1.0 / (double)(k_nano_second_in_one_second);

    public:
        SlamKernel(int32_t img_width, int32_t img_height, std::string vocabulary_data, int64_t begin_time_stamp);
        SlamKernel(const SlamKernel&) = delete;
        SlamKernel& operator=(const SlamKernel&) = delete;
        ~SlamKernel();

        TrackingResult handleData(const std::vector<Image>& images, const std::vector<ImuPoint>& imus);

        void reset();

    private:
        int32_t m_width;
        int32_t m_height;
        const int64_t m_begin_time_stamp;

        std::unique_ptr<::ORB_SLAM3::System> m_orb_slam;

        std::chrono::steady_clock::time_point m_last_time;
    };

}
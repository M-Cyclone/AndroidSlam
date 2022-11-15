#pragma once
#include <algorithm>
#include <numeric>

#include <glm/glm.hpp>

namespace android_slam
{

    static constexpr float k_float_max = std::numeric_limits<float>::max();
    static constexpr float k_float_min = std::numeric_limits<float>::min();

    struct AABB
    {
        glm::vec3 min_conner;
        glm::vec3 max_conner;

        AABB() noexcept
            : min_conner(k_float_max, k_float_max, k_float_max)
            , max_conner(k_float_min, k_float_min, k_float_min)
        {}
        AABB(const AABB&) noexcept = default;
        AABB& operator=(const AABB&) noexcept = default;

        void addPoint(glm::vec3 point) noexcept
        {
            min_conner.x = std::min(min_conner.x, point.x);
            min_conner.y = std::min(min_conner.y, point.y);
            min_conner.z = std::min(min_conner.z, point.z);
            max_conner.x = std::max(max_conner.x, point.x);
            max_conner.y = std::max(max_conner.y, point.y);
            max_conner.z = std::max(max_conner.z, point.z);
        }

        glm::vec3 getCenter() const noexcept
        {
            return (min_conner + max_conner) * 0.5f;
        }

        float getCubeMaxHalfLength() const noexcept
        {
            float edge = 0.0f;
            edge = std::max(edge, max_conner.x - min_conner.x);
            edge = std::max(edge, max_conner.y - min_conner.y);
            edge = std::max(edge, max_conner.z - min_conner.z);
            return edge * 0.5f;
        }
    };

}
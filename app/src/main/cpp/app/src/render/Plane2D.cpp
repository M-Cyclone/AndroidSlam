#include "render/Plane2D.h"

namespace android_slam
{

    const std::vector<Plane2D::Vertex> Plane2D::k_vertices
    {
        Vertex{ Eigen::Vector2f{ -1.0f, -1.0f }, Eigen::Vector2f{ 0.0f, 0.0f } },
        Vertex{ Eigen::Vector2f{ +1.0f, -1.0f }, Eigen::Vector2f{ 1.0f, 0.0f } },
        Vertex{ Eigen::Vector2f{ +1.0f, +1.0f }, Eigen::Vector2f{ 1.0f, 1.0f } },
        Vertex{ Eigen::Vector2f{ -1.0f, +1.0f }, Eigen::Vector2f{ 0.0f, 1.0f } }
    };

    const std::vector<uint32_t> Plane2D::k_indices =
    {
        0, 1, 2,
        0, 2, 3
    };

}
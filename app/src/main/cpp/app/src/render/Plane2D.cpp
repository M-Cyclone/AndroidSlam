#include "render/Plane2D.h"

namespace android_slam
{

const std::vector<Plane2D::Vertex> Plane2D::k_vertices
{
    Vertex{glm::vec2{ -1.0f, -1.0f }, glm::vec2{ 0.0f, 0.0f }},
    Vertex{glm::vec2{ +1.0f, -1.0f }, glm::vec2{ 1.0f, 0.0f }},
    Vertex{glm::vec2{ +1.0f, +1.0f }, glm::vec2{ 1.0f, 1.0f }},
    Vertex{glm::vec2{ -1.0f, +1.0f }, glm::vec2{ 0.0f, 1.0f }}
};

const std::vector<uint32_t> Plane2D::k_indices = { 0, 1, 2, 0, 2, 3 };

}  // namespace android_slam
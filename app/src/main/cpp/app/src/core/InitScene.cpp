#include "core/InitScene.h"

#include <glm/gtc/matrix_transform.hpp>

#include "core/App.h"

#include "utils/AssetManager.h"
#include "utils/ImageLoader.h"

namespace android_slam
{

    void InitScene::init()
    {
        m_image_painter = std::make_unique<Plane2D>();


        m_image = ImageLoader::createImage("images/school.image");
        int32_t img_width = m_image->getWidth();
        int32_t img_height = m_image->getHeight();


        m_image_shader = std::make_unique<Shader>("shader/image3d.vert", "shader/image3d.frag");


        float window_aspect_ratio = m_app.getWindow().getAspectRatio() * ((float)img_height / (float)img_width);
        m_proj = glm::perspective(glm::radians(90.0f), window_aspect_ratio, 0.1f, 10.0f);
        m_view = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void InitScene::exit()
    {
        m_image.reset(nullptr);
        m_image_shader.reset(nullptr);
    }

    void InitScene::update(float dt)
    {
        (void)dt;

        glViewport(0, 0, m_app.getWindow().getWidth(), m_app.getWindow().getHeight());

        m_image_painter->bind();
        m_image_shader->bind();

        m_image_shader->setMat4("u_mat_mvp", m_proj * m_view);

        glActiveTexture(GL_TEXTURE0);
        m_image_shader->setInt("u_image", 0);
        m_image->bind();

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void*)0);

        m_image->unbind();
        m_image_shader->unbind();
        m_image_painter->unbind();
    }

    void InitScene::drawGui(float dt)
    {
        if(ImGui::TreeNode("Options"))
        {
            if(ImGui::Button(m_initialized ? "Resume Tracking" :"Start Tracking"))
            {
                m_initialized = true;
                m_app.setActiveScene("Slam");
            }

            ImGui::TreePop();
        }
    }

}
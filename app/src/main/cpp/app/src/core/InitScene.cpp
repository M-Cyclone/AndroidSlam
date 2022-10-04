#include "core/InitScene.h"

#include "core/App.h"

namespace android_slam
{

    void InitScene::init()
    {

    }

    void InitScene::exit()
    {

    }

    void InitScene::update(float dt)
    {
        (void)dt;
    }

    void InitScene::drawGui(float dt)
    {
        if(ImGui::TreeNode("Options"))
        {
            if(ImGui::Button(m_initialized ? "Resume Tracking" :"Start Tracking"))
            {
                m_initialized = true;
                m_app.setActiveScene("slam");
            }

            ImGui::TreePop();
        }
    }

}
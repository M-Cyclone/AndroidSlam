#pragma once
#include <memory>

#include "render/ImageTexture.h"

namespace android_slam
{

    class ImageLoader
    {
    public:
        static std::unique_ptr<ImageTexture> createImage(const char* file);
    };

}
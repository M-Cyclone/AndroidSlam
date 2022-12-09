#pragma once
#include <memory>

#include "render/ImageTexture.h"

namespace android_slam
{

// 建议图像读取器，用于读取assets里的原始数据格式图像
class ImageLoader
{
public:
    static std::unique_ptr<ImageTexture> createImage(const char* file);
};

}  // namespace android_slam
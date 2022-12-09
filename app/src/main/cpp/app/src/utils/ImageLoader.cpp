#include "utils/ImageLoader.h"
#include <sstream>

#include "utils/AssetManager.h"
#include "utils/Log.h"

namespace android_slam
{
namespace image_loader_utils
{

typedef union
{
    int32_t i32;
    uint8_t ui8[4];
} bit32_t;

}  // namespace image_loader_utils

std::unique_ptr<ImageTexture> ImageLoader::createImage(const char* file)
{
    AAsset* asset = AAssetManager_open(AssetManager::get(), file, AASSET_MODE_BUFFER);
    assert(asset && "[Android Slam Shader Info] Failed to open shader file.");

    size_t size = AAsset_getLength(asset);
    auto   data = (const uint8_t*)AAsset_getBuffer(asset);

    image_loader_utils::bit32_t width;
    image_loader_utils::bit32_t height;
    memcpy(width.ui8, data, sizeof(image_loader_utils::bit32_t));
    memcpy(height.ui8, data + 4, sizeof(image_loader_utils::bit32_t));

    assert(size == (width.i32 * height.i32 * 3 + 8));


    std::vector<uint8_t> image_data(width.i32 * height.i32 * 3);
    memcpy(image_data.data(), data + 8, sizeof(uint8_t) * image_data.size());

    AAsset_close(asset);

    return std::make_unique<ImageTexture>(width.i32, height.i32, image_data);
}

}  // namespace android_slam
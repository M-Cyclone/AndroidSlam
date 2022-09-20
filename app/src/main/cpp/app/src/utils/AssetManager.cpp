#include "utils/AssetManager.h"
#include <cassert>

namespace android_slam
{

    AAssetManager* AssetManager::s_asset_manager = nullptr;

    bool AssetManager::getData(const std::string& path, int mode, std::vector<uint8_t>& data)
    {
        AAsset* asset = AAssetManager_open(s_asset_manager, path.c_str(), mode);

        if(!asset) return false;

        size_t size = AAsset_getLength(asset);
        data.resize(size);

        auto buffer = (const uint8_t*)AAsset_getBuffer(asset);
        AAsset_read(asset, data.data(), size);

        AAsset_close(asset);
        return true;
    }

}
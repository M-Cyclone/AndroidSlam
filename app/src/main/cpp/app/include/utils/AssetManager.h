#pragma once
#include <string>
#include <vector>

#include <android/asset_manager.h>

namespace android_slam
{

    class AssetManager
    {
    public:
        static bool getData(const std::string& path, int mode, std::vector<uint8_t>& data);

        static AAssetManager* get() { return s_asset_manager; }
        static void set(AAssetManager* manager) { s_asset_manager = manager; }

    private:
        static AAssetManager* s_asset_manager;
    };

}
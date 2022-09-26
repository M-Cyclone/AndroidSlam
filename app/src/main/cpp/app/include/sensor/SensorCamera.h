#pragma once
#include <memory>
#include <vector>

#include <media/NdkImage.h>
#include <media/NdkImageReader.h>

#include <camera/NdkCameraCaptureSession.h>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraError.h>
#include <camera/NdkCaptureRequest.h>

namespace android_slam
{

    class SensorCamera
    {
    private:
        using AndroidImage = std::unique_ptr<AImage, decltype(&AImage_delete)>;
        using AndroidImageReader = std::unique_ptr<AImageReader, decltype(&AImageReader_delete)>;

        using CameraManager = std::unique_ptr<ACameraManager, decltype(&ACameraManager_delete)>;
        using CameraIdList = std::unique_ptr<ACameraIdList, decltype(&ACameraManager_deleteCameraIdList)>;
        using CameraDevice = std::unique_ptr<ACameraDevice, decltype(&ACameraDevice_close)>;
        using CaptureSessionOutputContainer = std::unique_ptr<ACaptureSessionOutputContainer, decltype(&ACaptureSessionOutputContainer_free)>;
        using CaptureSessionOutput = std::unique_ptr<ACaptureSessionOutput, decltype(&ACaptureSessionOutput_free)>;
        using CameraCaptureSession = std::unique_ptr<ACameraCaptureSession, decltype(&ACameraCaptureSession_close)>;
        using CaptureRequest = std::unique_ptr<ACaptureRequest, decltype(&ACaptureRequest_free)>;
        using CameraOutputTarget = std::unique_ptr<ACameraOutputTarget, decltype(&ACameraOutputTarget_free)>;

        using CameraMetadata = std::unique_ptr<ACameraMetadata, decltype(&ACameraMetadata_free)>;

        static constexpr size_t k_max_image_count = 2;

    public:
        SensorCamera(int32_t width, int32_t height, int32_t format, uint64_t usage);
        SensorCamera(const SensorCamera&) = delete;
        SensorCamera& operator=(const SensorCamera&) = delete;

        void startCapture();
        void stopCapture();

        AImage* getLatestImage() const;

    private:
        mutable size_t m_curr_img_idx = 0;

        AndroidImageReader m_img_reader;
        ANativeWindow* m_img_reader_window = nullptr;
        mutable std::vector<AndroidImage> m_images;
        mutable std::vector<AImage*> m_image_buffers;

        CameraManager m_camera_manager;
        CameraIdList m_camera_id_list;
        CameraDevice m_camera_device;
        CaptureSessionOutputContainer m_capture_session_output_container;
        CaptureSessionOutput m_image_reader_output;
        CameraCaptureSession m_capture_session;
        CaptureRequest m_capture_request;
        CameraOutputTarget m_camera_output_target;

        ACameraDevice_StateCallbacks m_camera_device_state_callbacks;
        ACameraCaptureSession_stateCallbacks m_capture_session_state_callbacks;
    };

}
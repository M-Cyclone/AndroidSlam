#include "sensor/SensorCamera.h"
#include <string>
#include <thread>
#include <cassert>

#include "utils/Log.h"

namespace android_slam
{

    SensorCamera::SensorCamera(int32_t width, int32_t height, int32_t format, uint64_t usage)
        : m_img_reader{ nullptr, AImageReader_delete }
        , m_camera_manager{ nullptr, ACameraManager_delete }
        , m_camera_id_list{ nullptr, ACameraManager_deleteCameraIdList }
        , m_camera_device{ nullptr, ACameraDevice_close }
        , m_capture_session_output_container{ nullptr, ACaptureSessionOutputContainer_free }
        , m_image_reader_output{ nullptr, ACaptureSessionOutput_free }
        , m_capture_session{ nullptr, ACameraCaptureSession_close }
        , m_capture_request{nullptr, ACaptureRequest_free }
        , m_camera_output_target{ nullptr, ACameraOutputTarget_free }
        , m_camera_device_state_callbacks{}
        , m_capture_session_state_callbacks{}
    {
        media_status_t ms{};

        // Create images, and allocate image buffers.
        {
            for(size_t i = 0; i < k_max_image_count; ++i)
            {
                m_images.emplace_back(nullptr, AImage_delete);
            }
            m_image_buffers.resize(k_max_image_count, nullptr);
        }

        // Create ImageReader.
        {
            decltype(m_img_reader.get()) ptr;
            ms = AImageReader_newWithUsage(width, height, format, usage, k_max_image_count + 2, &ptr);
            assert((ms == AMEDIA_OK) && "[Android Slam Sensor Info] Failed to create ImageReader.");

            m_img_reader.reset(ptr);
            assert(m_img_reader);
        }

        //Create ImageReader's window.
        {
            ms = AImageReader_getWindow(m_img_reader.get(), &m_img_reader_window);
            assert((ms == AMEDIA_OK) && "[Android Slam Sensor Info] Failed to create ImageReader's Native Window.");
            assert(m_img_reader_window && "[Android Slam Sensor Info] ImageReader's Native Window is NULL.");
        }


        camera_status_t cs{};

        // Create camera manager;
        {
            m_camera_manager.reset(ACameraManager_create());
            assert(m_camera_manager && "[Android Slam Sensor Info] Failed to create camera manager.");
        }

        // Enumerate cameras;
        {
            decltype(m_camera_id_list.get()) ptr;
            cs = ACameraManager_getCameraIdList(m_camera_manager.get(), &ptr);
            assert((cs == ACAMERA_OK) && "[Android Slam Sensor Info] Failed to get camera id list.");

            m_camera_id_list.reset(ptr);
            assert(m_camera_id_list && "[Android Slam Sensor Info] CameraIdList is NULL.");
            assert((m_camera_id_list->numCameras > 0) && "[Android Slam Sensor Info] No available camera.");
        }

        // Choose one back facing camera
        std::string active_camera_id;
        {
            for (int32_t i = 0; i < m_camera_id_list->numCameras; ++i)
            {
                const char* id = m_camera_id_list->cameraIds[i];

                ACameraMetadata* ptr;
                cs = ACameraManager_getCameraCharacteristics(m_camera_manager.get(), id, &ptr);
                assert((cs == ACAMERA_OK) && "[Android Slam Sensor Info] Failed to get camera characteristics.");

                // Using RAII feature to delete ptr memory.
                CameraMetadata metadata{ ptr, ACameraMetadata_free };
                assert(metadata && "[Android Slam Sensor Info] Metadata is NULL.");

                ACameraMetadata_const_entry lens_facing{};
                cs = ACameraMetadata_getConstEntry(metadata.get(), ACAMERA_LENS_FACING, &lens_facing);
                assert((cs == ACAMERA_OK) && "[Android Slam Sensor Info] Failed to get camera lens info.");
                auto facing = static_cast<acamera_metadata_enum_android_lens_facing_t>(lens_facing.data.u8[0]);

                if(facing == ACAMERA_LENS_FACING_BACK)
                {
                    active_camera_id = id;
                    break;
                }
            }

            assert((!active_camera_id.empty()) && "[Android Slam Sensor Info] Failed to get an active camera.");
        }

        // Create camera device.
        {
            m_camera_device_state_callbacks.context = this;
            m_camera_device_state_callbacks.onDisconnected = [](void* context, ACameraDevice* device)
            {
                DEBUG_INFO("[Android Slam Camera Info] Camera(ID: %s) is disconnected.", ACameraDevice_getId(device));
            };
            m_camera_device_state_callbacks.onError = [](void* context, ACameraDevice* device, int error)
            {
                DEBUG_INFO("[Android Slam Camera Info] Camera(ID: %s)\nError Code: %d", ACameraDevice_getId(device), error);
            };

            decltype(m_camera_device.get()) ptr;
            cs = ACameraManager_openCamera(m_camera_manager.get(),
                                           active_camera_id.c_str(),
                                           &m_camera_device_state_callbacks,
                                           &ptr);
            while(cs == ACAMERA_ERROR_PERMISSION_DENIED)
            {
                static int count = 0;
                if(count == 10)
                {
                    assert(false && "[Android Slam Sensor Info] Run out of time when opening camera.");
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                cs = ACameraManager_openCamera(m_camera_manager.get(),
                                               active_camera_id.c_str(),
                                               &m_camera_device_state_callbacks,
                                               &ptr);
                ++count;
            }
            assert((cs == ACAMERA_OK) && "[Android Slam Sensor Info] Failed to open active camera.");

            m_camera_device.reset(ptr);
            assert(m_camera_device && "[Android Slam Sensor Info] CameraDevice is NULL.");
        }

        // Create capture session output container.
        {
            decltype(m_capture_session_output_container.get()) ptr;
            cs = ACaptureSessionOutputContainer_create(&ptr);
            assert((cs == ACAMERA_OK) && "[Android Slam Sensor Info] Failed to create capture session output container.");

            m_capture_session_output_container.reset(ptr);
            assert(m_capture_session_output_container && "[Android Slam Sensor Info] Capture Session Output Container is NULL.");
        }

        //  Create capture session output, which is the place to save camera images.
        {
            decltype(m_image_reader_output.get()) ptr;
            cs = ACaptureSessionOutput_create(m_img_reader_window, &ptr);
            assert((cs == ACAMERA_OK) && "[Android Slam Sensor Info] Failed to create capture session output.");

            m_image_reader_output.reset(ptr);
            assert(m_image_reader_output && "[Android Slam Sensor Info] ImageReaderOutput is NULL.");
        }

        // Add image reader output to output container.
        {
            cs = ACaptureSessionOutputContainer_add(m_capture_session_output_container.get(), m_image_reader_output.get());
            assert((cs == ACAMERA_OK) && "[Android Slam Sensor Info] Failed to add ImageReaderOutput to CaptureSessionOutputContainer.");
        }

        // Create capture session.
        {
            m_capture_session_state_callbacks.context = this;
            m_capture_session_state_callbacks.onClosed = [](void* context, ACameraCaptureSession* session)
            {
                DEBUG_INFO("[Android Slam Camera Info] Session(ptr == %p) is closed.", session);
            };
            m_capture_session_state_callbacks.onReady = [](void* context, ACameraCaptureSession* session)
            {
                DEBUG_INFO("[Android Slam Camera Info] Session(ptr == %p) is ready.", session);
            };
            m_capture_session_state_callbacks.onActive = [](void* context, ACameraCaptureSession* session)
            {
                DEBUG_INFO("[Android Slam Camera Info] Session(ptr == %p) is activated.", session);
            };

            decltype(m_capture_session.get()) ptr;
            cs = ACameraDevice_createCaptureSession(m_camera_device.get(),
                                                    m_capture_session_output_container.get(),
                                                    &m_capture_session_state_callbacks,
                                                    &ptr);
            assert((cs == ACAMERA_OK) && "[Android Slam Sensor Info] Failed to create capture session.");

            m_capture_session.reset(ptr);
            assert(m_capture_session && "[Android Slam Sensor Info] CaptureSession is NULL.");
        }

        // Create capture request.
        {
            decltype(m_capture_request.get()) ptr;
            cs = ACameraDevice_createCaptureRequest(m_camera_device.get(), TEMPLATE_PREVIEW, &ptr);
            assert((cs == ACAMERA_OK) && "[Android Slam Sensor Info] Failed to create capture request.");

            m_capture_request.reset(ptr);
            assert(m_capture_request && "[Android Slam Sensor Info] CaptureRequest is NULL.");
        }

        // Create camera output target.
        {
            decltype(m_camera_output_target.get()) ptr;
            cs = ACameraOutputTarget_create(m_img_reader_window, &ptr);
            assert((cs == ACAMERA_OK) && "[Android Slam Sensor Info] Failed to create camera output target.");

            m_camera_output_target.reset(ptr);
            assert(m_camera_output_target && "[Android Slam Sensor Info] CameraOutputTarget is NULL.");
        }

        // Add target to capture request.
        {
            cs = ACaptureRequest_addTarget(m_capture_request.get(), m_camera_output_target.get());
            assert((cs == ACAMERA_OK) && "[Android Slam Sensor Info] Failed to add camera output target to capture request.");
        }
    }

    void SensorCamera::startCapture()
    {
        std::vector<ACaptureRequest*> capture_request_list{ m_capture_request.release() };
        camera_status_t cs = ACameraCaptureSession_setRepeatingRequest(
                m_capture_session.get(),
                nullptr,
                (int)capture_request_list.size(),
                capture_request_list.data(),
                nullptr
                );
        assert((cs == ACAMERA_OK) && "[Android Slam Sensor Info] Failed to set repeating request.");

        m_capture_request.reset(capture_request_list[0]);
        assert(m_capture_request && "[Android Slam Sensor Info] Capture Request is NULL when start capturing.");


        while(!getLatestImage())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000u));
        }
        assert(getLatestImage());

        DEBUG_INFO("[Android Slam Sensor Info] Camera Start Capturing.");
    }

    void SensorCamera::stopCapture()
    {
        ACameraCaptureSession_stopRepeating(m_capture_session.get());

        DEBUG_INFO("[Android Slam Sensor Info] Camera Stop Capturing.");
    }

    AImage* SensorCamera::getLatestImage() const
    {
        AImage* image = nullptr;
        media_status_t ms = AImageReader_acquireLatestImage(m_img_reader.get(), &image);
        if(ms == AMEDIA_OK && image)
        {
            m_curr_img_idx = (m_curr_img_idx + 1) % k_max_image_count;
            m_images[m_curr_img_idx].reset(image);
            m_image_buffers[m_curr_img_idx] = image;
        }

        return m_image_buffers[m_curr_img_idx];
    }

    std::vector<uint8_t> SensorCamera::getLatestRGBImage() const
    {
        AImage* ndk_img = getLatestImage();
        assert(ndk_img && "[Android Slam Sensor Info] NDK Image access nullptr.");

        media_status_t ms{};

        std::vector<uint8_t> rgb_image{};

        uint8_t* y_pixel;
        uint8_t* u_pixel;
        uint8_t* v_pixel;
        int32_t y_len;
        int32_t u_len;
        int32_t v_len;

        ms = AImage_getPlaneData(ndk_img, 0, &y_pixel, &y_len);
        assert((ms == AMEDIA_OK) && "[Android Slam Sensor Info] Failed to get plane data.");
        ms = AImage_getPlaneData(ndk_img, 1, &u_pixel, &u_len);
        assert((ms == AMEDIA_OK) && "[Android Slam Sensor Info] Failed to get plane data.");
        ms = AImage_getPlaneData(ndk_img, 2, &v_pixel, &v_len);
        assert((ms == AMEDIA_OK) && "[Android Slam Sensor Info] Failed to get plane data.");



    }

}
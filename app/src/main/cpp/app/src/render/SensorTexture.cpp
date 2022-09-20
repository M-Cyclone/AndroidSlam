#include "render/SensorTexture.h"

namespace android_slam
{

    PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC SensorTexture::eglGetNativeClientBufferANDROID;
    PFNEGLCREATEIMAGEKHRPROC SensorTexture::eglCreateImageKHR;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC SensorTexture::glEGLImageTargetTexture2DOES;

    void SensorTexture::setImage(AImage* image)
    {
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, m_texture);
        AHardwareBuffer* hardware_buffer;
        media_status_t ms = AImage_getHardwareBuffer(image, &hardware_buffer);
        assert((ms == AMEDIA_OK) && "[Android Slam Render Info] Failed to get hardware buffer from AImage.");

        EGLClientBuffer client_buffer = eglGetNativeClientBufferANDROID(hardware_buffer);

        EGLint image_attributes[] = { EGL_NONE };
        EGLImageKHR egl_image_khr = eglCreateImageKHR(
                eglGetCurrentDisplay(),
                EGL_NO_CONTEXT,
                EGL_NATIVE_BUFFER_ANDROID,
                client_buffer,
                image_attributes
                );

        glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, egl_image_khr);

        m_curr_image = image;
    }

    void SensorTexture::registerFunctions()
    {
#define REGISTER_EXT_FUNC(type, func) func = reinterpret_cast<type>(eglGetProcAddress(#func))
        REGISTER_EXT_FUNC(PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC, eglGetNativeClientBufferANDROID);
        REGISTER_EXT_FUNC(PFNEGLCREATEIMAGEKHRPROC, eglCreateImageKHR);
        REGISTER_EXT_FUNC(PFNGLEGLIMAGETARGETTEXTURE2DOESPROC, glEGLImageTargetTexture2DOES);
#undef REGISTER_EXT_FUNC
    }

}
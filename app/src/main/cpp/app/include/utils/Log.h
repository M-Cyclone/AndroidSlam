#pragma once
#include <iostream>
#include <streambuf>

#include <android/log.h>

namespace android_slam
{

// 重定向到std::cout所使用的安卓缓冲区
class AndroidCOutBuffer : public ::std::streambuf
{
protected:
    static constexpr const size_t k_max_buffer_size = 1024 - 1;

public:
    AndroidCOutBuffer() noexcept
    {
        m_buffer[k_max_buffer_size] = '\0';
        setp(m_buffer, m_buffer + k_max_buffer_size - 1);
    }
    virtual ~AndroidCOutBuffer() noexcept { sync(); }

    virtual int sync() override
    {
        flush_buffer();
        return 0;
    }

protected:
    virtual int_type overflow(int_type __c = traits_type::eof()) override
    {
        if (__c != EOF)
        {
            *pptr() = __c;
            pbump(1);
        }
        flush_buffer();
        return __c;
    }

private:
    int flush_buffer()
    {
        int len = int(pptr() - pbase());
        if (len <= 0)
        {
            return 0;
        }

        if (len <= k_max_buffer_size)
        {
            m_buffer[len] = '\0';
        }

        __android_log_write(ANDROID_LOG_INFO, "[Android SLAM]", m_buffer);

        pbump(-len);
        return len;
    }

private:
    char m_buffer[k_max_buffer_size + 1];
};

}  // namespace android_slam
#include "sensor/SensorIMU.h"
#include <cassert>
#include <iostream>
#include <sstream>

namespace android_slam
{

SensorIMU::SensorIMU()
{
    m_sensor_manager = ASensorManager_getInstanceForPackage(k_package_name);
    assert(m_sensor_manager && "Failed to get sensor manager.");

    m_looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    assert(m_looper && "Failed to prepare looper.");


    m_accelerometer = ASensorManager_getDefaultSensor(m_sensor_manager, ASENSOR_TYPE_ACCELEROMETER);
    assert(m_accelerometer && "Failed to get accelerometer sensor.");

    m_gyroscope = ASensorManager_getDefaultSensor(m_sensor_manager, ASENSOR_TYPE_GYROSCOPE);
    assert(m_gyroscope && "Failed to get gyroscope sensor.");


    m_sensor_event_queue = ASensorManager_createEventQueue(m_sensor_manager, m_looper, 3, nullptr, nullptr);
    assert(m_sensor_event_queue && "Failed to create sensor event queue.");


    int32_t status;

    status = ASensorEventQueue_enableSensor(m_sensor_event_queue, m_accelerometer);
    assert((status >= 0) && "Failed to enable accelerometer sensor.");
    status = ASensorEventQueue_setEventRate(m_sensor_event_queue, m_accelerometer, k_sensor_refresh_delta_time_us);
    assert((status >= 0) && "Failed to enable accelerometer sensor.");

    status = ASensorEventQueue_enableSensor(m_sensor_event_queue, m_gyroscope);
    assert((status >= 0) && "Failed to enable gyroscope sensor.");
    status = ASensorEventQueue_setEventRate(m_sensor_event_queue, m_gyroscope, k_sensor_refresh_delta_time_us);
    assert((status >= 0) && "Failed to enable gyroscope sensor.");
}

SensorIMU::~SensorIMU() noexcept
{
    ASensorManager_destroyEventQueue(m_sensor_manager, m_sensor_event_queue);
}

std::vector<ImuPoint> SensorIMU::getImuData() const
{
    ALooper_pollAll(0, nullptr, nullptr, nullptr);
    ASensorEvent sensor_event;

    while (ASensorEventQueue_getEvents(m_sensor_event_queue, &sensor_event, 1) > 0)
    {
        if (sensor_event.type == ASENSOR_TYPE_ACCELEROMETER)
        {
            m_acce_data_que.emplace(sensor_event.acceleration.x,
                                    sensor_event.acceleration.y,
                                    sensor_event.acceleration.z,
                                    sensor_event.timestamp);
        }
        else if (sensor_event.type == ASENSOR_TYPE_GYROSCOPE)
        {
            m_gyro_data_que.emplace(sensor_event.gyro.x,
                                    sensor_event.gyro.y,
                                    sensor_event.gyro.z,
                                    sensor_event.timestamp);
        }
    }

    size_t count = std::min(m_acce_data_que.size(), m_gyro_data_que.size());

    std::vector<ImuPoint> imu_data;
    imu_data.reserve(count);

    for (size_t i = 0; i < count; ++i)
    {
        auto [ax, ay, az, at] = m_acce_data_que.front();
        auto [wx, wy, wz, wt] = m_gyro_data_que.front();

        m_acce_data_que.pop();
        m_gyro_data_que.pop();

        imu_data.emplace_back(ax, ay, az, wx, wy, wz, wt);
    }

    return imu_data;
}

}  // namespace android_slam
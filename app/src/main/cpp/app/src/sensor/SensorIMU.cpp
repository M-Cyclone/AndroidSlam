#include "sensor/SensorIMU.h"
#include <cassert>

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

        m_accelerometer_event_queue = ASensorManager_createEventQueue(m_sensor_manager, m_looper, 3, nullptr, nullptr);
        assert(m_accelerometer_event_queue && "Failed to create accelerometer event queue.");


        m_gyroscope = ASensorManager_getDefaultSensor(m_sensor_manager, ASENSOR_TYPE_GYROSCOPE);
        assert(m_gyroscope && "Failed to get gyroscope sensor.");

        m_gyroscope_event_queue = ASensorManager_createEventQueue(m_sensor_manager, m_looper, 3, nullptr, nullptr);
        assert(m_gyroscope_event_queue && "Failed to create gyroscope event queue.");


        int32_t status;

        status = ASensorEventQueue_enableSensor(m_accelerometer_event_queue, m_accelerometer);
        assert((status >= 0) && "Failed to enable accelerometer sensor.");
        status = ASensorEventQueue_setEventRate(m_accelerometer_event_queue, m_accelerometer, k_sensor_refresh_delta_time_us);
        assert((status >= 0) && "Failed to enable accelerometer sensor.");

        status = ASensorEventQueue_enableSensor(m_gyroscope_event_queue, m_gyroscope);
        assert((status >= 0) && "Failed to enable gyroscope sensor.");
        status = ASensorEventQueue_setEventRate(m_gyroscope_event_queue, m_gyroscope, k_sensor_refresh_delta_time_us);
        assert((status >= 0) && "Failed to enable gyroscope sensor.");
    }

    SensorIMU::~SensorIMU() noexcept
    {
        ASensorManager_destroyEventQueue(m_sensor_manager, m_accelerometer_event_queue);
        ASensorManager_destroyEventQueue(m_sensor_manager, m_gyroscope_event_queue);
    }

    std::vector<ImuPoint> SensorIMU::getImuData() const
    {
        ALooper_pollAll(0, nullptr, nullptr, nullptr);

        ASensorEvent sensor_event;

        while (ASensorEventQueue_getEvents(m_accelerometer_event_queue, &sensor_event, 1) > 0)
        {
            m_acce_data_que.emplace(sensor_event.acceleration.x,
                                    sensor_event.acceleration.y,
                                    sensor_event.acceleration.z,
                                    sensor_event.timestamp);
        }

        while (ASensorEventQueue_getEvents(m_gyroscope_event_queue, &sensor_event, 1) > 0)
        {
            m_gyro_data_que.emplace(sensor_event.acceleration.x,
                                    sensor_event.acceleration.y,
                                    sensor_event.acceleration.z,
                                    sensor_event.timestamp);
        }


        std::vector<ImuPoint> imu_data;

        if(m_acce_data_que.front().time_stamp < m_gyro_data_que.front().time_stamp)
        {
            if(m_acce_data_que.size() < 2) return imu_data;

            AcceData last_acce = m_acce_data_que.front();
            m_acce_data_que.pop();
            AcceData curr_acce = m_acce_data_que.front();

            while(!m_acce_data_que.empty() && curr_acce.time_stamp < m_gyro_data_que.front().time_stamp)
            {
                last_acce = curr_acce;
                m_acce_data_que.pop();
                curr_acce = m_acce_data_que.front();
            }
            assert(last_acce.time_stamp < m_gyro_data_que.front().time_stamp);
            assert(m_gyro_data_que.front().time_stamp <= curr_acce.time_stamp);


            GyroData curr_gyro{};

            while(!m_acce_data_que.empty() && !m_gyro_data_que.empty())
            {
                curr_acce = m_acce_data_que.front();
                curr_gyro = m_gyro_data_que.front();
                m_acce_data_que.pop();
                m_gyro_data_que.pop();


                float ratio = (float)(curr_gyro.time_stamp - last_acce.time_stamp) / (float)(curr_acce.time_stamp - last_acce.time_stamp);

                float ax = last_acce.ax * (1.0f - ratio) + curr_acce.ax * ratio;
                float ay = last_acce.ay * (1.0f - ratio) + curr_acce.ay * ratio;
                float az = last_acce.az * (1.0f - ratio) + curr_acce.az * ratio;

                imu_data.emplace_back(ax, ay, az, curr_gyro.wx, curr_gyro.wy, curr_gyro.wz, curr_gyro.time_stamp);
            }
        }
        else
        {
            if(m_gyro_data_que.size() < 2) return imu_data;

            GyroData last_gyro = m_gyro_data_que.front();
            m_gyro_data_que.pop();
            GyroData curr_gyro = m_gyro_data_que.front();

            while(!m_gyro_data_que.empty() && curr_gyro.time_stamp < m_acce_data_que.front().time_stamp)
            {
                last_gyro = curr_gyro;
                m_gyro_data_que.pop();
                curr_gyro = m_gyro_data_que.front();
            }
            assert(last_gyro.time_stamp < m_acce_data_que.front().time_stamp);
            assert(m_acce_data_que.front().time_stamp <= curr_gyro.time_stamp);


            AcceData curr_acce{};

            while(!m_acce_data_que.empty() && !m_gyro_data_que.empty())
            {
                curr_gyro = m_gyro_data_que.front();
                curr_acce = m_acce_data_que.front();
                m_gyro_data_que.pop();
                m_acce_data_que.pop();


                float ratio = (float)(curr_acce.time_stamp - last_gyro.time_stamp) / (float)(curr_gyro.time_stamp - last_gyro.time_stamp);

                float wx = last_gyro.wx * (1.0f - ratio) + curr_gyro.wx * ratio;
                float wy = last_gyro.wy * (1.0f - ratio) + curr_gyro.wy * ratio;
                float wz = last_gyro.wz * (1.0f - ratio) + curr_gyro.wz * ratio;

                imu_data.emplace_back(curr_acce.ax, curr_acce.ay, curr_acce.az, wx, wy, wz, curr_acce.time_stamp);
            }
        }

        return imu_data;
    }

}
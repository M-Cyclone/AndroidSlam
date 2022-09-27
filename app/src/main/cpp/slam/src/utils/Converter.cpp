/**
* This file is part of ORB-SLAM3
*
* Copyright (C) 2017-2021 Carlos Campos, Richard Elvira, Juan J. Gómez Rodríguez, José M.M. Montiel and Juan D. Tardós, University of Zaragoza.
* Copyright (C) 2014-2016 Raúl Mur-Artal, José M.M. Montiel and Juan D. Tardós, University of Zaragoza.
*
* ORB-SLAM3 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM3 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
* the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with ORB-SLAM3.
* If not, see <http://www.gnu.org/licenses/>.
*/

#include "utils/Converter.h"

namespace ORB_SLAM3
{

std::vector<cv::Mat> Converter::toDescriptorVector(const cv::Mat &Descriptors)
{
    std::vector<cv::Mat> vDesc;
    vDesc.reserve(Descriptors.rows);
    for (int j = 0; j < Descriptors.rows; j++)
    {
        vDesc.push_back(Descriptors.row(j));
    }

    return vDesc;
}

g2o::SE3Quat Converter::toSE3Quat(const cv::Mat &cvT)
{
    assert(cvT.rows == 3);
    assert(cvT.cols == 3);

    Eigen::Matrix<double,3,3> R;
    R << cvT.at<float>(0,0), cvT.at<float>(0,1), cvT.at<float>(0,2),
         cvT.at<float>(1,0), cvT.at<float>(1,1), cvT.at<float>(1,2),
         cvT.at<float>(2,0), cvT.at<float>(2,1), cvT.at<float>(2,2);

    Eigen::Matrix<double,3,1> t(cvT.at<float>(0,3), cvT.at<float>(1,3), cvT.at<float>(2,3));

    return { R, t };
}

g2o::SE3Quat Converter::toSE3Quat(const Sophus::SE3f& T)
{
    return { T.unit_quaternion().cast<double>(), T.translation().cast<double>() };
}

cv::Mat Converter::toCvMat(const g2o::SE3Quat &SE3)
{
    Eigen::Matrix<double,4,4> eigMat = SE3.to_homogeneous_matrix();
    return toCvMat(eigMat);
}

cv::Mat Converter::toCvMat(const g2o::Sim3 &Sim3)
{
    const Eigen::Matrix3d& eigR = Sim3.rotation().toRotationMatrix();
    const Eigen::Vector3d& eigt = Sim3.translation();

    double s = Sim3.scale();

    return toCvSE3(s * eigR, eigt);
}

cv::Mat Converter::toCvMat(const Eigen::Matrix<double,4,4>& m)
{
    cv::Mat cvMat(4, 4, CV_32F);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            cvMat.at<float>(i, j) = (float)m(i, j);
        }
    }

    return cvMat.clone();
}

cv::Mat Converter::toCvMat(const Eigen::Matrix<float,4,4> &m)
{
    cv::Mat cvMat(4, 4, CV_32F);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            cvMat.at<float>(i, j) = m(i, j);
        }
    }

    return cvMat.clone();
}

cv::Mat Converter::toCvMat(const Eigen::Matrix<float, 3, 4> &m)
{
    cv::Mat cvMat(3, 4, CV_32F);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            cvMat.at<float>(i, j) = m(i, j);
        }
    }

    return cvMat.clone();
}

cv::Mat Converter::toCvMat(const Eigen::Matrix3d &m)
{
    cv::Mat cvMat(3, 3, CV_32F);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            cvMat.at<float>(i, j) = (float)m(i, j);
        }
    }

    return cvMat.clone();
}

cv::Mat Converter::toCvMat(const Eigen::Matrix3f &m)
{
    cv::Mat cvMat(3, 3, CV_32F);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            cvMat.at<float>(i, j) = m(i, j);
        }
    }

    return cvMat.clone();
}

cv::Mat Converter::toCvMat(const Eigen::MatrixXf &m)
{
    cv::Mat cvMat((int)m.rows(), (int)m.cols(), CV_32F);
    for (int i = 0; i < m.rows(); i++)
    {
        for (int j = 0; j < m.cols(); j++)
        {
            cvMat.at<float>(i, j) = m(i, j);
        }
    }

    return cvMat.clone();
}

cv::Mat Converter::toCvMat(const Eigen::MatrixXd &m)
{
    cv::Mat cvMat((int) m.rows(), (int) m.cols(), CV_32F);
    for (int i = 0; i < m.rows(); i++)
    {
        for (int j = 0; j < m.cols(); j++)
        {
            cvMat.at<float>(i, j) = (float)m(i, j);
        }
    }

    return cvMat.clone();
}

cv::Mat Converter::toCvMat(const Eigen::Matrix<double,3,1> &m)
{
    cv::Mat cvMat(3, 1, CV_32F);
    for (int i = 0; i < 3; i++)
    {
        cvMat.at<float>(i) = (float)m(i);
    }

    return cvMat.clone();
}

cv::Mat Converter::toCvMat(const Eigen::Matrix<float,3,1> &m)
{
    cv::Mat cvMat(3, 1, CV_32F);
    for (int i = 0; i < 3; i++)
    {
        cvMat.at<float>(i) = (float)m(i);
    }

    return cvMat.clone();
}

cv::Mat Converter::toCvSE3(const Eigen::Matrix<double,3,3> &R, const Eigen::Matrix<double,3,1> &t)
{
    cv::Mat cvMat = cv::Mat::eye(4, 4, CV_32F);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            cvMat.at<float>(i, j) = (float)R(i, j);
        }
    }
    for (int i = 0; i < 3; i++)
    {
        cvMat.at<float>(i, 3) = (float)t(i);
    }

    return cvMat.clone();
}

Eigen::Matrix<double,3,1> Converter::toVector3d(const cv::Mat &cvVector)
{
    assert(cvVector.rows == 3);
    assert(cvVector.cols == 1);

    Eigen::Matrix<double, 3, 1> v;
    v << cvVector.at<float>(0), cvVector.at<float>(1), cvVector.at<float>(2);

    return v;
}

Eigen::Matrix<float,3,1> Converter::toVector3f(const cv::Mat &cvVector)
{
    assert(cvVector.rows == 3);
    assert(cvVector.cols == 1);

    Eigen::Matrix<float, 3, 1> v;
    v << cvVector.at<float>(0), cvVector.at<float>(1), cvVector.at<float>(2);

    return v;
}

Eigen::Matrix<double,3,1> Converter::toVector3d(const cv::Point3f &cvPoint)
{
    Eigen::Matrix<double, 3, 1> v;
    v << cvPoint.x, cvPoint.y, cvPoint.z;

    return v;
}

Eigen::Matrix<double,3,3> Converter::toMatrix3d(const cv::Mat &cvMat3)
{
    assert(cvMat3.rows == 3);
    assert(cvMat3.cols == 3);

    Eigen::Matrix<double, 3, 3> M;

    M << cvMat3.at<float>(0, 0), cvMat3.at<float>(0, 1), cvMat3.at<float>(0, 2),
         cvMat3.at<float>(1, 0), cvMat3.at<float>(1, 1), cvMat3.at<float>(1, 2),
         cvMat3.at<float>(2, 0), cvMat3.at<float>(2, 1), cvMat3.at<float>(2, 2);

    return M;
}

Eigen::Matrix<double,4,4> Converter::toMatrix4d(const cv::Mat &cvMat4)
{
    assert(cvMat4.rows == 4);
    assert(cvMat4.cols == 4);

    Eigen::Matrix<double, 4, 4> M;

    M << cvMat4.at<float>(0, 0), cvMat4.at<float>(0, 1), cvMat4.at<float>(0, 2), cvMat4.at<float>(0, 3),
         cvMat4.at<float>(1, 0), cvMat4.at<float>(1, 1), cvMat4.at<float>(1, 2), cvMat4.at<float>(1, 3),
         cvMat4.at<float>(2, 0), cvMat4.at<float>(2, 1), cvMat4.at<float>(2, 2), cvMat4.at<float>(2, 3),
         cvMat4.at<float>(3, 0), cvMat4.at<float>(3, 1), cvMat4.at<float>(3, 2), cvMat4.at<float>(3, 3);

    return M;
}

Eigen::Matrix<float,3,3> Converter::toMatrix3f(const cv::Mat &cvMat3)
{
    assert(cvMat3.rows == 3);
    assert(cvMat3.cols == 3);

    Eigen::Matrix<float,3,3> M;

    M << cvMat3.at<float>(0,0), cvMat3.at<float>(0,1), cvMat3.at<float>(0,2),
         cvMat3.at<float>(1,0), cvMat3.at<float>(1,1), cvMat3.at<float>(1,2),
         cvMat3.at<float>(2,0), cvMat3.at<float>(2,1), cvMat3.at<float>(2,2);

    return M;
}

Eigen::Matrix<float,4,4> Converter::toMatrix4f(const cv::Mat &cvMat4)
{
    assert(cvMat4.rows == 4);
    assert(cvMat4.cols == 4);

    Eigen::Matrix<float, 4, 4> M;

    M << cvMat4.at<float>(0, 0), cvMat4.at<float>(0, 1), cvMat4.at<float>(0, 2), cvMat4.at<float>(0, 3),
         cvMat4.at<float>(1, 0), cvMat4.at<float>(1, 1), cvMat4.at<float>(1, 2), cvMat4.at<float>(1, 3),
         cvMat4.at<float>(2, 0), cvMat4.at<float>(2, 1), cvMat4.at<float>(2, 2), cvMat4.at<float>(2, 3),
         cvMat4.at<float>(3, 0), cvMat4.at<float>(3, 1), cvMat4.at<float>(3, 2), cvMat4.at<float>(3, 3);

    return M;
}

std::vector<float> Converter::toQuaternion(const cv::Mat &M)
{
    assert(M.rows == 3);
    assert(M.cols == 3);

    Eigen::Matrix<double,3,3> eigMat = toMatrix3d(M);
    Eigen::Quaterniond q(eigMat);

    std::vector<float> v
    {
        (float)q.x(),
        (float)q.y(),
        (float)q.z(),
        (float)q.w()
    };

    return v;
}

cv::Mat Converter::tocvSkewMatrix(const cv::Mat &v)
{
    assert(v.rows == 3);
    assert(v.cols == 1);

    return static_cast<cv::Mat>(cv::Mat_<float>(3, 3) <<
        0, -v.at<float>(2), v.at<float>(1),
        v.at<float>(2), 0, -v.at<float>(0),
        -v.at<float>(1), v.at<float>(0), 0
    );
}

bool Converter::isRotationMatrix(const cv::Mat &R)
{
    assert(R.rows == 3);
    assert(R.cols == 3);

    cv::Mat Rt;
    cv::transpose(R, Rt);
    cv::Mat shouldBeIdentity = Rt * R;
    cv::Mat I = cv::Mat::eye(3, 3, shouldBeIdentity.type());

    return cv::norm(I, shouldBeIdentity) < 1e-6;
}

std::vector<float> Converter::toEuler(const cv::Mat &R)
{
    assert(R.rows == 3);
    assert(R.cols == 3);
    assert(isRotationMatrix(R));

    float sy = std::sqrt(R.at<float>(0, 0) * R.at<float>(0, 0) + R.at<float>(1, 0) * R.at<float>(1, 0));

    float x, y, z;
    if (sy >= 1e-6) // singular
    {
        x = atan2(R.at<float>(2, 1), R.at<float>(2, 2));
        y = atan2(-R.at<float>(2, 0), sy);
        z = atan2(R.at<float>(1, 0), R.at<float>(0, 0));
    }
    else
    {
        x = atan2(-R.at<float>(1, 2), R.at<float>(1, 1));
        y = atan2(-R.at<float>(2, 0), sy);
        z = 0;
    }

    return { x, y, z };
}

Sophus::SE3<float> Converter::toSophus(const cv::Mat &T)
{
    assert(T.rows == 4);
    assert(T.cols == 4);

    Eigen::Matrix<double, 3, 3> eigMat = toMatrix3d(T.rowRange(0, 3).colRange(0, 3));
    Eigen::Quaternionf q(eigMat.cast<float>());

    Eigen::Matrix<float, 3, 1> t = toVector3d(T.rowRange(0, 3).col(3)).cast<float>();

    return { q, t };
}

Sophus::Sim3f Converter::toSophus(const g2o::Sim3& S)
{
    return
    {
        Sophus::RxSO3d((float) S.scale(), S.rotation().matrix()).cast<float>(),
        S.translation().cast<float>()
    };
}

} //namespace ORB_SLAM

// This file is part of the AliceVision project.
// Copyright (c) 2016 AliceVision contributors.
// Copyright (c) 2012 openMVG contributors.
// Copyright (c) 2007 libmv contributors.
// This Source Code Form is subject to the terms of the Mozilla Public License,
// v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

// AliceVision does not support Eigen with alignment, unless C++17 aligned new feature is enabled.
// So ensure Eigen is used with the correct flags.
#ifndef SWIG
#ifndef ALICEVISION_EIGEN_REQUIRE_ALIGNMENT
    #ifndef EIGEN_MAX_ALIGN_BYTES
        #error "EIGEN_MAX_ALIGN_BYTES is not defined"
    #elif EIGEN_MAX_ALIGN_BYTES != 0
        #error "EIGEN_MAX_ALIGN_BYTES is defined but not 0"
    #endif

    #ifndef EIGEN_MAX_STATIC_ALIGN_BYTES
        #error "EIGEN_MAX_STATIC_ALIGN_BYTES is not defined"
    #elif EIGEN_MAX_STATIC_ALIGN_BYTES != 0
        #error "EIGEN_MAX_STATIC_ALIGN_BYTES is defined but not 0"
    #endif
#endif
#endif

//--
// Eigen
// http://eigen.tuxfamily.org/dox-devel/QuickRefPage.html
//--
#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <Eigen/Geometry>
#include <Eigen/LU>
#include <Eigen/QR>
#include <Eigen/SparseCore>
#include <Eigen/SVD>
#include <Eigen/StdVector>
#include <boost/math/constants/constants.hpp>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <string>
#include <iostream>
#include <vector>

namespace aliceVision {

// Check MSVC
#if _WIN32 || _WIN64
    #if _WIN64
        #define ENV64BIT
    #else
        #define ENV32BIT
    #endif
#endif

// Check GCC
#if __GNUC__
    #if __x86_64__ || __ppc64__ || _LP64
        #define ENV64BIT
    #else
        #define ENV32BIT
    #endif
#endif

using Eigen::Map;

using EigenDoubleTraits = Eigen::NumTraits<double>;

using Vec3 = Eigen::Vector3d;
using Vec3i = Eigen::Vector3i;
using Vec3f = Eigen::Vector3f;

using Vec2i = Eigen::Vector2i;
using Vec2f = Eigen::Vector2f;

using Vec9 = Eigen::Matrix<double, 9, 1>;

using Quaternion = Eigen::Quaternion<double>;

using Mat3 = Eigen::Matrix<double, 3, 3>;

#if defined(ENV32BIT)
using Mat23 = Eigen::Matrix<double, 2, 3, Eigen::DontAlign>;
using Mat34 = Eigen::Matrix<double, 3, 4, Eigen::DontAlign>;
using Vec2 = Eigen::Matrix<double, 2, 1, Eigen::DontAlign>;
using Vec4 = Eigen::Matrix<double, 4, 1, Eigen::DontAlign>;
using Vec6 = Eigen::Matrix<double, 6, 1, Eigen::DontAlign>;
#else  // 64 bits compiler
using Mat23 = Eigen::Matrix<double, 2, 3>;
using Mat34 = Eigen::Matrix<double, 3, 4>;
using Vec2 = Eigen::Vector2d;
using Vec4 = Eigen::Vector4d;
using Vec6 = Eigen::Matrix<double, 6, 1>;
#endif

using Mat4 = Eigen::Matrix<double, 4, 4>;
using Matu = Eigen::Matrix<unsigned int, Eigen::Dynamic, Eigen::Dynamic>;

using RMat3 = Eigen::Matrix<double, 3, 3, Eigen::RowMajor>;

//-- General purpose Matrix and Vector
using Mat = Eigen::MatrixXd;
using Vec = Eigen::VectorXd;
using Vecu = Eigen::Matrix<unsigned int, Eigen::Dynamic, 1>;
using Matf = Eigen::MatrixXf;
using Vecf = Eigen::VectorXf;
using Vecb = Eigen::Matrix<bool, Eigen::Dynamic, 1>;

using Mat2X = Eigen::Matrix<double, 2, Eigen::Dynamic>;
using Mat3X = Eigen::Matrix<double, 3, Eigen::Dynamic>;
using Mat4X = Eigen::Matrix<double, 4, Eigen::Dynamic>;

using MatX9 = Eigen::Matrix<double, Eigen::Dynamic, 9>;
using Mat9 = Eigen::Matrix<double, 9, 9>;

//-- Sparse Matrix (Column major, and row major)
using sMat = Eigen::SparseMatrix<double>;
using sRMat = Eigen::SparseMatrix<double, Eigen::RowMajor>;

//--------------
//-- Function --
//--------------

/// Return the square of a number.

template<typename T>
inline T Square(T x)
{
    return x * x;
}

/// Clamp return the number if inside range, else min or max range.

template<typename T>
inline T clamp(const T& val, const T& min, const T& max)
{
    return std::max(min, std::min(val, max));
    //(val < min) ? val : ((val>max) ? val : max);
}

inline bool isSimilar(double a, double b)
{
    const double diff = a - b;
    return std::abs(diff) < 1e-8;
}
inline bool isSimilar(float a, float b)
{
    const float diff = a - b;
    return std::abs(diff) < 1e-8f;
}

/**
 * @brief Create a minimal skew matrix from a 2d vector.
 * @param[in] x A 2d vector whose 3rd coordinate is supposed to be 1.
 * @return The minimal ske matrix: [0, -1, x(1); 1, 0, -x(0);]
 */
Mat23 SkewMatMinimal(const Vec2& x);

/**
 * @brief Create a cross product matrix from a 3d vector.
 * @param x A 3d vector.
 * @return the cross matrix representation of the input vector.
 */
Mat3 CrossProductMatrix(const Vec3& x);

// Create a rotation matrix around axis X with the provided radian angle
Mat3 RotationAroundX(double angle);

// Create a rotation matrix around axis Y with the provided radian angle
Mat3 RotationAroundY(double angle);

// Create a rotation matrix around axis Z with the provided radian angle
Mat3 RotationAroundZ(double angle);

Mat3 rotationXYZ(double angleX, double angleY, double angleZ);

// Degree to Radian (suppose input in [0;360])
template<typename T>
inline T degreeToRadian(T degree)
{
    static_assert(std::is_floating_point<T>::value, "degreeToRadian: must be floating point.");
    return degree * boost::math::constants::pi<T>() / 180.0;
}

// Radian to degree
template<typename T>
inline T radianToDegree(T radian)
{
    static_assert(std::is_floating_point<T>::value, "radianToDegree: must be floating point.");
    return radian / boost::math::constants::pi<T>() * 180.0;
}

/// Return in radian the mean rotation amplitude of the given rotation matrix
/// Computed as the mean of matrix column dot products to an Identity matrix
double getRotationMagnitude(const Mat3& R2);

/**
 * @brief Compute the angle between two rotation matrices.
 * @param[in] R1 The first rotation matrix.
 * @param[in] R2 The second rotation matrix.
 * @return The angle between the two rotations as the angle of the rotation
 * matrix R1*R2.transpose().
 */
double rotationDifference(const Mat3& R1, const Mat3& R2);

inline double SIGN(double x) { return x < 0.0 ? -1.0 : 1.0; }

// L1 norm = Sum (|x0| + |x1| + |xn|)

template<typename TVec>
inline double NormL1(const TVec& x)
{
    return x.array().abs().sum();
}

// L2 norm = Sqrt (Sum (x0^2 + x1^2 + xn^2))

template<typename TVec>
inline double NormL2(const TVec& x)
{
    return x.norm();
}

// LInfinity norm = max (|x0|, |x1|, ..., |xn|)

template<typename TVec>
inline double NormLInfinity(const TVec& x)
{
    return x.array().abs().maxCoeff();
}

template<typename TVec>
inline double DistanceL1(const TVec& x, const TVec& y)
{
    return (x - y).array().abs().sum();
}

template<typename TVec>
inline double DistanceL2(const TVec& x, const TVec& y)
{
    return (x - y).norm();
}

template<typename TVec>
inline double DistanceLInfinity(const TVec& x, const TVec& y)
{
    return NormLInfinity(x - y);
}

template<typename TVec>
inline bool AreVecNearEqual(const TVec& x, const TVec& y, const double epsilon)
{
    assert(x.cols() == y.cols());
    for (typename TVec::Index i = 0; i < x.cols(); ++i)
    {
        if ((y(i) - epsilon > x(i)) || (x(i) > y(i) + epsilon))
            return false;
    }
    return true;
}

template<typename TMat>
inline bool AreMatNearEqual(const TMat& X, const TMat& Y, const double epsilon)
{
    assert(X.cols() == Y.cols());
    assert(X.rows() == Y.rows());
    for (typename TMat::Index i = 0; i < X.rows(); ++i)
    {
        for (typename TMat::Index j = 0; j < X.cols(); ++j)
        {
            if ((Y(i, j) - epsilon > X(i, j)) || (X(i, j) > Y(i, j) + epsilon))
                return false;
        }
    }
    return true;
}

// Make a rotation matrix such that center becomes the direction of the
// positive z-axis, and y is oriented close to up by default.
Mat3 LookAt(const Vec3& center, const Vec3& up = Vec3::UnitY());

Mat3 LookAt2(const Vec3& eyePosition3D, const Vec3& center3D = Vec3::Zero(), const Vec3& upVector3D = Vec3::UnitY());

#define SUM_OR_DYNAMIC(x, y) (x == Eigen::Dynamic || y == Eigen::Dynamic) ? Eigen::Dynamic : (x + y)

template<typename Derived1, typename Derived2>
struct hstack_return
{
    using Scalar = typename Derived1::Scalar;

    enum
    {
        RowsAtCompileTime = Derived1::RowsAtCompileTime,
        ColsAtCompileTime = SUM_OR_DYNAMIC(Derived1::ColsAtCompileTime, Derived2::ColsAtCompileTime),
        Options = Derived1::Flags & Eigen::RowMajorBit ? Eigen::RowMajor : 0,
        MaxRowsAtCompileTime = Derived1::MaxRowsAtCompileTime,
        MaxColsAtCompileTime = SUM_OR_DYNAMIC(Derived1::MaxColsAtCompileTime, Derived2::MaxColsAtCompileTime)
    };
    typedef Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime, Options, MaxRowsAtCompileTime, MaxColsAtCompileTime> type;
};

template<typename Derived1, typename Derived2>
typename hstack_return<Derived1, Derived2>::type HStack(const Eigen::MatrixBase<Derived1>& lhs, const Eigen::MatrixBase<Derived2>& rhs)
{
    typename hstack_return<Derived1, Derived2>::type res;
    res.resize(lhs.rows(), lhs.cols() + rhs.cols());
    res << lhs, rhs;
    return res;
}

template<typename Derived1, typename Derived2>
struct vstack_return
{
    using Scalar = typename Derived1::Scalar;

    enum
    {
        RowsAtCompileTime = SUM_OR_DYNAMIC(Derived1::RowsAtCompileTime, Derived2::RowsAtCompileTime),
        ColsAtCompileTime = Derived1::ColsAtCompileTime,
        Options = Derived1::Flags & Eigen::RowMajorBit ? Eigen::RowMajor : 0,
        MaxRowsAtCompileTime = SUM_OR_DYNAMIC(Derived1::MaxRowsAtCompileTime, Derived2::MaxRowsAtCompileTime),
        MaxColsAtCompileTime = Derived1::MaxColsAtCompileTime
    };
    typedef Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime, Options, MaxRowsAtCompileTime, MaxColsAtCompileTime> type;
};

template<typename Derived1, typename Derived2>
typename vstack_return<Derived1, Derived2>::type VStack(const Eigen::MatrixBase<Derived1>& lhs, const Eigen::MatrixBase<Derived2>& rhs)
{
    typename vstack_return<Derived1, Derived2>::type res;
    res.resize(lhs.rows() + rhs.rows(), lhs.cols());
    res << lhs, rhs;
    return res;
}
#undef SUM_OR_DYNAMIC

template<typename TMat>
inline double FrobeniusNorm(const TMat& A)
{
    return A.norm();
}

template<typename TMat>
inline double FrobeniusDistance(const TMat& A, const TMat& B)
{
    return FrobeniusNorm(A - B);
}

template<class TMat>
double CosinusBetweenMatrices(const TMat& a, const TMat& b)
{
    return (a.array() * b.array()).sum() / FrobeniusNorm(a) / FrobeniusNorm(b);
}

/**
 * @brief Given a vector of element and a vector containing a selection of its indices,
 * it returns a new vector containing only the selected elements of the original vector
 * @tparam T The type of data contained in the vector.
 * @param[out] result The output vector containing only the selected original elements.
 * @param[in] input The input vector.
 * @param[in] selection The vector containing the selection of elements of \p input
 * through the indices specified in \p selection.
 */
template<typename T>
void pick(std::vector<T>& result, const std::vector<T>& input, const std::vector<typename std::vector<T>::size_type>& selection)
{
    result.reserve(selection.size());
    std::transform(
      selection.begin(), selection.end(), std::back_inserter(result), [&input](typename std::vector<T>::size_type idx) { return input.at(idx); });
}

void MeanAndVarianceAlongRows(const Mat& A, Vec* mean_pointer, Vec* variance_pointer);

bool exportMatToTextFile(const Mat& mat, const std::string& filename, const std::string& sPrefix = "A");

inline int is_finite(const double val)
{
#ifdef _MSC_VER
    return _finite(val);
#else
    return std::isfinite(val);
#endif
}

/**
 ** Split a range [ a ; b [ into a set of n ranges :
 [ a ; c1 [ U [ c1 ; c2 [ U ... U [ c(n-1) ; b [
 **
  Output range vector only store [ a , c1 , c2 , ... , b ]

 ** if input range can't be split (range [a;b[ size is less than nb_split, only return [a;b[ range
 **
 ** @param range_start Start of range to split
 ** @param range_end End of range to split
 ** @param nb_split Number of desired split
 ** @param d_range Output splitted range
 **/
template<typename T>
void SplitRange(const T range_start, const T range_end, const int nb_split, std::vector<T>& d_range)
{
    const T range_length = range_end - range_start;
    if (range_length < nb_split)
    {
        d_range.push_back(range_start);
        d_range.push_back(range_end);
    }
    else
    {
        const T delta_range = range_length / nb_split;

        d_range.push_back(range_start);
        for (int i = 1; i < nb_split; ++i)
        {
            d_range.push_back(range_start + i * delta_range);
        }
        d_range.push_back(range_end);
    }
}

template<class T>
constexpr T divideRoundUp(T x, T y)
{
    static_assert(std::is_integral<T>::value, "divideRoundUp only works with integer arguments");
    assert(y != 0);  // Prevents division by zero
    const auto xPos = x >= 0;
    const auto yPos = y >= 0;
    if (xPos == yPos)
    {
        return x / y + T((x % y) != 0);
    }
    else
    {
        // negative result, rounds towards zero anyways
        return x / y;
    }
}

/**
 * This function initializes the global state of random number generators that e.g. our tests
 * depend on. This makes it possible to have exactly reproducible program runtime behavior
 * without random changes. In order to introduce variation in execution,
 * ALICEVISION_RANDOM_SEED environment variable can be set to an integer value.
 *
 * This function is especially useful in tests where it allows to prevent random failures.
 */
void makeRandomOperationsReproducible();

}  // namespace aliceVision

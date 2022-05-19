#pragma once
#ifndef __NR_BOUNDS_HPP__
#define __NR_BOUNDS_HPP__

#include <algorithm>
#include <limits>
#include "geometry/vec.hpp"

namespace NRenderer {
using namespace std;
inline Vec3 minVec(const Vec3& v1, const Vec3& v2) {
    return {fmin(v1.x, v2.x), fmin(v1.y, v2.y), fmin(v1.z, v2.z)};
}

inline Vec3 maxVec(const Vec3& v1, const Vec3& v2) {
    return {fmax(v1.x, v2.x), fmax(v1.y, v2.y), fmax(v1.z, v2.z)};
}
class Bounds {
   public:
    Vec3 minPoint, maxPoint;
    Bounds() {
        float min = numeric_limits<float>::lowest();
        float max = numeric_limits<float>::max();
        maxPoint = {min, min, min};
        minPoint = {max, max, max};
    }

    Bounds(const Vec3& v) : minPoint(v), maxPoint(v) {}

    Bounds(const Vec3& v1, const Vec3& v2) {
        minPoint = {fmin(v1.x, v2.x), fmin(v1.y, v2.y), fmin(v1.z, v2.z)};
        maxPoint = {fmax(v1.x, v2.x), fmax(v1.y, v2.y), fmax(v1.z, v2.z)};
    }

    Vec3 getDiagonal() const { return maxPoint - minPoint; }

    int getMaxAxis() const {
        Vec3 d = getDiagonal();
        if (d.x > d.y && d.x > d.z)
            return 0;
        else if (d.y > d.z)
            return 1;
        else
            return 2;
    }

    float getArea() const {
        Vec3 d = getDiagonal();
        return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
    }

    Vec3 getCentroid() const { return (minPoint + maxPoint) * 0.5f; }
    Bounds getIntersection(const Bounds& b) {
        return Bounds(maxVec(minPoint, b.minPoint),
                      minVec(maxPoint, b.maxPoint));
    }

    Vec3 getOffset(const Vec3& p) {
        Vec3 off = p - minPoint;
        if (maxPoint.x > minPoint.x)
            off.x /= maxPoint.x - minPoint.x;
        if (maxPoint.y > minPoint.y)
            off.y /= maxPoint.y - minPoint.y;
        if (maxPoint.z > minPoint.z)
            off.z /= maxPoint.z - minPoint.z;
    }

    static bool isOverlap(const Bounds& b1, const Bounds& b2) {
#define TEST(x) \
    (b1.maxPoint.x >= b2.minPoint.x) && (b1.minPoint.x <= b2.maxPoint.x)
        return TEST(x) && TEST(y) && TEST(z);
    }

    static bool isInside(const Vec3& v, const Bounds& b) {
#define TEST1(x) (v.x >= b.minPoint.x) && (v.x <= b.maxPoint.x)
        return TEST1(x) && TEST1(y) && TEST1(z);
    }

    static inline Bounds Union(const Bounds& b1, const Bounds& b2) {
        Bounds ret;
        ret.minPoint = minVec(b1.minPoint, b2.minPoint);
        ret.maxPoint = maxVec(b1.maxPoint, b2.maxPoint);
        return ret;
    }

    static inline Bounds Union(const Bounds& b, const Vec3& v) {
        Bounds ret;
        ret.minPoint = minVec(b.minPoint, v);
        ret.maxPoint = maxVec(b.maxPoint, v);
        return ret;
    }

    inline const Vec3& operator[](int i) const {
        if (i == 0)
            return minPoint;
        if (i == 1)
            return maxPoint;
        return {};
    }
};
}  // namespace NRenderer
#endif __NR_BOUNDS_HPP__
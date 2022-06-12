#include "intersections/intersections.hpp"
#include <array>

namespace PhotonMapping::Intersection {
HitRecord xTriangle(const Ray& ray, const Triangle& t, float tMin, float tMax) {
    const auto& v1 = t.v1;
    const auto& v2 = t.v2;
    const auto& v3 = t.v3;
    const auto& normal = t.normal;
    auto e1 = v2 - v1;
    auto e2 = v3 - v1;
    auto P = glm::cross(ray.direction, e2);
    float det = glm::dot(e1, P);
    Vec3 T;
    if (det > 0)
        T = ray.origin - v1;
    else {
        T = v1 - ray.origin;
        det = -det;
    }
    if (det < 0.000001f)
        return getMissRecord();
    // 重心坐标
    float u, v, w;
    u = glm::dot(T, P);
    if (u > det || u < 0.f)
        return getMissRecord();
    Vec3 Q = glm::cross(T, e1);
    v = glm::dot(ray.direction, Q);
    if (v < 0.f || v + u > det)
        return getMissRecord();
    w = glm::dot(e2, Q);
    float invDet = 1.f / det;
    w *= invDet;
    if (w >= tMax || w < tMin)
        return getMissRecord();
    return getHitRecord(w, ray.at(w), normal, t.material);
}

// 如果有两个交点 只会返回先交的交点
HitRecord xSphere(const Ray& ray, const Sphere& s, float tMin, float tMax) {
    const auto& position = s.position;
    const auto& r = s.radius;
    Vec3 oc = ray.origin - position;
    float a = glm::dot(ray.direction, ray.direction);
    float b = glm::dot(oc, ray.direction);
    float c = glm::dot(oc, oc) - r * r;
    float discriminant = b * b - a * c;
    float sqrtDiscriminant = sqrt(discriminant);
    if (discriminant > 0) {
        float temp = (-b - sqrtDiscriminant) / a;
        if (temp < tMax && temp >= tMin) {
            auto hitPoint = ray.at(temp);
            auto normal = (hitPoint - position) / r;
            return getHitRecord(temp, hitPoint, normal, s.material);
        }
        temp = (-b + sqrtDiscriminant) / a;
        if (temp < tMax && temp >= tMin) {
            auto hitPoint = ray.at(temp);
            auto normal = (hitPoint - position) / r;
            return getHitRecord(temp, hitPoint, normal, s.material);
        }
    }
    return getMissRecord();
}

HitRecord xPlane(const Ray& ray, const Plane& p, float tMin, float tMax) {
    auto Np_dot_d = glm::dot(ray.direction, p.normal);
    if (Np_dot_d < 0.0000001f && Np_dot_d > -0.00000001f)
        return getMissRecord();
    // dp实际上是 Ax+By+Cz+D=0 中的 D
    float dp = -glm::dot(p.position, p.normal);
    float t = (-dp - glm::dot(p.normal, ray.origin)) / Np_dot_d;
    if (t >= tMax || t < tMin)
        return getMissRecord();
    // cross test
    Vec3 hitPoint = ray.at(t);
    Vec3 normal = p.normal;
    Mat3x3 d{p.u, p.v, glm::cross(p.u, p.v)};
    d = glm::inverse(d);
    auto res = d * (hitPoint - p.position);
    auto u = res.x, v = res.y;
    if ((u <= 1 && u >= 0) && (v <= 1 && v >= 0)) {
        return getHitRecord(t, hitPoint, normal, p.material);
    }
    return getMissRecord();
}

// 所以相当于与平面求交 通过uv坐标判断是否与光源区域相交吗？
HitRecord xAreaLight(const Ray& ray,
                     const AreaLight& a,
                     float tMin,
                     float tMax) {
    Vec3 normal = glm::cross(a.u, a.v);
    Vec3 position = a.position;
    auto Np_dot_d = glm::dot(ray.direction, normal);
    if (Np_dot_d < 0.0000001f && Np_dot_d > -0.00000001f)
        return getMissRecord();
    float dp = -glm::dot(position, normal);
    float t = (-dp - glm::dot(normal, ray.origin)) / Np_dot_d;
    if (t >= tMax || t < tMin)
        return getMissRecord();
    // cross test
    Vec3 hitPoint = ray.at(t);
    Mat3x3 d{a.u, a.v, glm::cross(a.u, a.v)};
    d = glm::inverse(d);
    auto res = d * (hitPoint - position);
    auto u = res.x, v = res.y;
    if ((u <= 1 && u >= 0) && (v <= 1 && v >= 0)) {
        return getHitRecord(t, hitPoint, normal, {});
    }
    return getMissRecord();
}

bool xBound(const Ray& ray, const Bounds& b) {
    Vec3 dir = ray.direction;
    Vec3 dirInv = {1.0f / dir.x, 1.0f / dir.y, 1.0f / dir.z};
    Vec3 tMin = (b.minPoint - ray.origin) * dirInv;
    Vec3 tMax = (b.maxPoint - ray.origin) * dirInv;
    if (dir.x < 0)
        swap(tMin.x, tMax.x);
    if (dir.y < 0)
        swap(tMin.y, tMax.y);
    if (dir.z < 0)
        swap(tMin.z, tMax.z);
    float enter = max({tMin.x, tMin.y, tMin.z});
    float exit = min({tMax.x, tMax.y, tMax.z});
    if (enter <= exit && exit > 0)
        return true;
    return false;
}

HitRecord xBVH(const Ray& ray, BVHNode* node) {
    HitRecord closestHit = nullopt;
    if (!xBound(ray, node->bound))
        return closestHit;
    if (!node->left && !node->right) {
        switch (node->object->type) {
            case Entity::EntityType::SPHERE:
                closestHit = Intersection::xSphere(
                    ray, *(Sphere*)(node->object), 0.001, FLOAT_INF);
                break;
            case Entity::EntityType::PLANE:
                closestHit = Intersection::xPlane(ray, *(Plane*)(node->object),
                                                  0.000001, FLOAT_INF);
                break;
            case Entity::EntityType::TRIANGLE:
                closestHit = Intersection::xTriangle(
                    ray, *(Triangle*)(node->object), 0.000001, FLOAT_INF);
                break;
            case Entity::EntityType::MESH:
                closestHit = Intersection::xMesh(ray, *(Mesh*)(node->object),
                                                 0.000001, FLOAT_INF);
                break;
        }
        return closestHit;
    }
    auto temp1 = xBVH(ray, node->left);
    auto temp2 = xBVH(ray, node->right);
    if (!temp1)
        return temp2;
    if (!temp2)
        return temp1;
    return temp1->t < temp2->t ? temp1 : temp2;
}

HitRecord closestHitObject(BVH* bvh, const Ray& ray) {
    HitRecord closestHit = nullopt;
    if (!bvh->root)
        return closestHit;
    closestHit = xBVH(ray, bvh->root);
    return closestHit;
}

HitRecord xMesh(const Ray& ray, const Mesh& p, float tMin, float tMax) {
    HitRecord closestHit = nullopt;
    if (p.bvh)
        closestHit = closestHitObject(p.bvh, ray);
    if (!closestHit || closestHit->t >= tMax || closestHit->t < tMin)
        return getMissRecord();
    return closestHit;
}

}  // namespace PhotonMapping::Intersection
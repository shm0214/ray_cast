#pragma once
#ifndef __NR_MESH_H__
#define __NR_MESH_H__

#include <vector>
#include "BVH.hpp"
#include "Model.hpp"

namespace NRenderer {
using namespace std;
struct Mesh : public Entity {
    vector<Vec3> normals;
    vector<Vec3> positions;
    vector<Vec2> uvs;
    vector<Index> normalIndices;
    vector<Index> positionIndices;
    vector<Index> uvIndices;
    BVH* bvh;
    Mesh() { type = EntityType::MESH; }

    bool hasNormal() const { return normals.size() != 0; }

    bool hasUv() const { return uvs.size() != 0; }

    void calBoundingBox() {
        float min = numeric_limits<float>::lowest();
        float max = numeric_limits<float>::max();
        Vec3 minV(min, min, min), maxV(max, max, max);
        for (auto v : positions) {
            minV = minVec(v, minV);
            maxV = maxVec(v, maxV);
        }
        boundingBox = Bounds(minV, maxV);
    }

    void calArea() {
        area = 0;
        for (int i = 0; i < positionIndices.size(); i += 3) {
            Vec3 v1 = positions[positionIndices[3 * i]];
            Vec3 v2 = positions[positionIndices[3 * i + 1]];
            Vec3 v3 = positions[positionIndices[3 * i + 2]];
            area += glm::length(glm::cross(v2 - v1, v3 - v1)) / 2;
        }
    }

    void buildBVH() {
        vector<Entity*> triangles;
        hasArea = true;
        area = 0;
        // 先不管法向量了
        for (int i = 0; i < positionIndices.size(); i += 3) {
            Vec3 v1 = positions[positionIndices[3 * i]];
            Vec3 v2 = positions[positionIndices[3 * i + 1]];
            Vec3 v3 = positions[positionIndices[3 * i + 2]];
            area += glm::length(glm::cross(v2 - v1, v3 - v1)) / 2;
            triangles.push_back(new Triangle(v1, v2, v3));
        }
        bvh = new BVH(triangles);
    }
};
SHARE(Mesh);
}  // namespace NRenderer
#endif __NR_MESH_H__
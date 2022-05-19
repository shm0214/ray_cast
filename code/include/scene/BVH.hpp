#pragma once
#ifndef __NR_BVH_H__
#define __NR_BVH_H__

#include <algorithm>
#include <vector>
#include "Bounds.hpp"
#include "Model.hpp"

namespace NRenderer {
using namespace std;
struct BVHNode {
    Bounds bound;
    BVHNode *left, *right;
    Entity* object;
    float area;
    int splitAxis = 0;
    int primNum = 0;
    int firstPrimOff = 0;

    BVHNode() {
        bound = Bounds();
        left = nullptr;
        right = nullptr;
        object = nullptr;
    }
};

class BVH {
   public:
    vector<Entity*> primitives;
    BVHNode* root;

    void sort(vector<Entity*> primitives, int dim) {
        switch (dim) {
            case 0:
                std::sort(primitives.begin(), primitives.end(),
                          [](auto p1, auto p2) {
                              return p1->getBoundingBox().getCentroid().x <
                                     p2->getBoundingBox().getCentroid().x;
                          });
                break;
            case 1:
                std::sort(primitives.begin(), primitives.end(),
                          [](auto p1, auto p2) {
                              return p1->getBoundingBox().getCentroid().y <
                                     p2->getBoundingBox().getCentroid().y;
                          });
                break;
            case 2:
                std::sort(primitives.begin(), primitives.end(),
                          [](auto p1, auto p2) {
                              return p1->getBoundingBox().getCentroid().z <
                                     p2->getBoundingBox().getCentroid().z;
                          });
                break;
        }
    }

    BVH(vector<Entity*> primitives) {
        if (primitives.empty())
            return;
        root = build(primitives);
        this->primitives = move(primitives);
    }
    BVHNode* build(vector<Entity*> primitives) {
        BVHNode* node = new BVHNode();
        Bounds bound;
        for (auto p : primitives)
            bound = Bounds::Union(bound, p->getBoundingBox());
        if (primitives.size() == 1) {
            node->bound = primitives[0]->getBoundingBox();
            node->object = primitives[0];
            node->left = nullptr;
            node->right = nullptr;
            node->area = primitives[0]->getArea();
            return node;
        } else if (primitives.size() == 2) {
            node->left = build(vector{primitives[0]});
            node->right = build(vector{primitives[1]});
            node->bound = Bounds::Union(node->left->bound, node->right->bound);
            node->area = node->left->area + node->right->area;
            return node;
        } else {
            auto begin = primitives.begin();
            auto end = primitives.end();
            int size = primitives.size();
            if (size < 12) {
                Bounds centroidBound;
                for (auto p : primitives)
                    centroidBound = Bounds::Union(
                        centroidBound, p->getBoundingBox().getCentroid());
                int axis = centroidBound.getMaxAxis();
                sort(primitives, axis);
                auto middle = begin + size / 2;
                auto left = vector<Entity*>(begin, middle);
                auto right = vector<Entity*>(middle, end);
                node->left = build(left);
                node->right = build(right);
                node->bound =
                    Bounds::Union(node->left->bound, node->right->bound);
                node->area = node->left->area + node->right->area;
            } else {
                auto middle = begin;
                int dim = 0, mid = 0;
                float cost = (numeric_limits<float>::max)();
                float nums[] = {1.0 / 6, 2.0 / 6, 3.0 / 6, 4.0 / 6, 5.0 / 6};
                for (int tmpDim = 0; tmpDim < 3; tmpDim++) {
                    sort(primitives, tmpDim);
                    Bounds left, right;
                    for (auto n : nums) {
                        int tmpMid = int(n * size);
                        for (int i = 0; i < tmpMid; i++) {
                            left = Bounds::Union(
                                left, primitives[i]->getBoundingBox());
                        }
                        for (int i = tmpMid; i < size; i++) {
                            right = Bounds::Union(
                                right, primitives[i]->getBoundingBox());
                        }
                        float tmpCost = (tmpMid * left.getArea() +
                                         (size - tmpMid) * right.getArea()) /
                                        bound.getArea();
                        if (tmpCost < cost) {
                            cost = tmpCost;
                            dim = tmpDim;
                            mid = tmpMid;
                        }
                    }
                }
                sort(primitives, dim);
                middle = begin + mid;
                auto left = vector<Entity*>(begin, middle);
                auto right = vector<Entity*>(middle, end);
                node->left = build(left);
                node->right = build(right);
                node->bound =
                    Bounds::Union(node->left->bound, node->right->bound);
                node->area = node->left->area + node->right->area;
            }
        }
        return node;
    }
};
}  // namespace NRenderer
#endif __NR_BVH_H__
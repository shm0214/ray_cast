#pragma once
#ifndef __PHOTON_HEAP_H__
#define __PHOTON_HEAP_H__

#include <vector>
#include "photon.hpp"

namespace PhotonMapping {
using namespace NRenderer;
using namespace std;

class PhotonHeap {
   public:
    int num;
    int maxNum;
    Vec3 position;
    int maxDist = 10000;
    vector<Photon> photons;
    vector<float> distances;
    PhotonHeap() {
        maxNum = 100;
        num = 0;
    }
    void setMaxDist(int dist) { maxDist = dist; }
    PhotonHeap(int maxNum) : maxNum(maxNum) { num = 0; }
    bool isFull() { return num == maxNum; }
    void addPhoton(Photon& p) {
        auto d2 = distance2(p.position - position);
        if (d2 > maxDist * maxDist) {
            cout << d2 << endl;
            return;
        }
        if (!isFull()) {
            num++;
            photons.push_back(p);
            distances.push_back(d2);
        } else {
            if (d2 > distances[0])
                return;
            photons[0] = p;
            distances[0] = d2;
        }
        for (int i = num / 2; i >= 0; i--)
            balance(i);
    }
    void balance(int idx) {
        int leftIdx = 2 * idx + 1;
        int rightIdx = 2 * idx + 2;
        if (leftIdx < num) {
            int p = distances[idx];
            int l = distances[leftIdx];
            if (rightIdx < num) {
                int r = distances[rightIdx];
                if (p >= l && p >= r) {
                    return;
                } else {
                    if (l > p && l >= r) {
                        swap(photons[leftIdx], photons[idx]);
                        swap(distances[leftIdx], distances[idx]);
                        balance(leftIdx);
                    } else if (r > p && r > l) {
                        swap(photons[rightIdx], photons[idx]);
                        swap(distances[rightIdx], distances[idx]);
                        balance(rightIdx);
                    }
                }
            } else {
                if (l > p) {
                    swap(photons[leftIdx], photons[idx]);
                    swap(distances[leftIdx], distances[idx]);
                }
            }
        }
    }
};
}  // namespace PhotonMapping

#endif
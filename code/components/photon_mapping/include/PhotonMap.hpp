#ifndef __PHOTON_MAP_H__
#define __PHOTON_MAP_H__

#include <vector>
#include "Photon.hpp"
#include "PhotonHeap.hpp"
#include "geometry/vec.hpp"
#include "scene/Bounds.hpp"

namespace PhotonMapping {
using namespace NRenderer;
using namespace std;

constexpr float Pi = 3.1415926535898f;

class PhotonMap {
    int num = 0;
    int maxNum;
    vector<Photon> photons;
    Bounds bound;

   public:
    PhotonMap() : maxNum(500000) {}
    PhotonMap(int maxNum) : maxNum(maxNum) {}
    void addPhoton(Photon photon) {
        if (num >= maxNum)
            return;
        photons.push_back(photon);
        bound = Bounds::Union(bound, photon.position);
        num++;
    }
    int getMaxNum() { return maxNum; }
    void balance() {
        vector<Photon> tempVec(photons);
        balanceSegment(tempVec, 1, 0, num - 1);
    }
    int calMid(int start, int end) {
        int num = end - start + 1;
        int tmp = 1;
        while (2 * tmp <= num)
            tmp *= 2;
        int mid = tmp / 2 - 1;
        num -= tmp - 1;
        if (num > tmp / 2)
            mid += tmp / 2;
        else
            mid += num;
        return start + mid;
    }
    void balanceSegment(vector<Photon>& vec, int idx, int start, int end) {
        if (start == end) {
            photons[idx - 1] = vec[start];
            photons[idx - 1].axis = -1;
            return;
        }
        int mid = calMid(start, end);
        int axis = bound.getMaxAxis();
        split(vec, start, end, mid, axis);
        photons[idx - 1] = vec[mid];
        photons[idx - 1].axis = axis;
        if (start < mid) {
            auto temp = bound.maxPoint[axis];
            bound.maxPoint[axis] = photons[idx - 1].position[axis];
            balanceSegment(vec, idx * 2, start, mid - 1);
            bound.maxPoint[axis] = temp;
        }
        if (mid < end) {
            auto temp = bound.minPoint[axis];
            bound.minPoint[axis] = photons[idx - 1].position[axis];
            balanceSegment(vec, idx * 2 + 1, mid + 1, end);
            bound.minPoint[axis] = temp;
        }
    }
    void split(vector<Photon>& vec, int start, int end, int mid, int axis) {
        while (start < end) {
            auto n = vec[end].position[axis];
            int i = start, j = end - 1;
            while (i < j) {
                while (vec[i].position[axis] < n)
                    i++;
                while (vec[j].position[axis] > n && j > i)
                    j--;
                if (i >= j)
                    break;
                swap(vec[i], vec[j]);
                i++;
                j--;
            }
            swap(vec[i], vec[end]);
            if (i >= mid)
                end = i - 1;
            if (i <= mid)
                start = i + 1;
        }
    }
    void getHeap(int idx, PhotonHeap& heap) {
        if (idx >= num)
            return;
        Photon p = photons[idx];
        if (p.axis != -1) {
            auto d1 = heap.position[p.axis] - p.position[p.axis];
            if (d1 < 0) {
                getHeap(idx * 2 + 1, heap);
                if (heap.distances.size() && d1 * d1 < heap.distances[0])
                    getHeap(idx * 2 + 2, heap);
            } else {
                getHeap(idx * 2 + 2, heap);
                if (heap.distances.size() && d1 * d1 < heap.distances[0])
                    getHeap(idx * 2 + 1, heap);
            }
        }
        heap.addPhoton(p);
    }
    void testBalance() {
        for (int i = 0; i < 10; i++) {
            Vec3 dir = defaultSamplerInstance<HemiSphere>().sample3d();
            Photon p(Vec3{dir[0] * 10, dir[1] * 10, dir[2] * 10}, Vec3{0},
                     Vec3{0});
            addPhoton(p);
        }
        balance();
        PhotonHeap p(5);
        p.position = Vec3{3, 2, 1};
        for (int i = 0; i < 10; i++) {
            auto t = photons[i];
            cout << t.position << " " << distance2(t.position - p.position)
                 << endl;
        }
        cout << endl;
        getHeap(0, p);
        for (int i = 0; i < 5; i++) {
            auto t = p.photons[i];
            cout << t.position << " " << distance2(t.position - p.position)
                 << endl;
        }
    }
    RGB getIrradiance(Vec3 position, Vec3 normal, float maxDist, int maxNum) {
        RGB ret{0};
        PhotonHeap p(maxNum);
        p.position = position;
        //p.setMaxDist(maxDist);
        getHeap(0, p);
        if (p.num <= 8)
            return {};
        for (int i = 0; i < p.num; i++) {
            auto direction = p.photons[i].direction;
            if (glm::dot(normal, direction) < 0)
                ret += p.photons[i].power;
        }
        ret = ret / (Pi * p.distances[0]);
        return ret;
    }
};
}  // namespace PhotonMapping

#endif
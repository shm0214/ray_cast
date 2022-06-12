#ifndef __PHOTON_H__
#define __PHOTON_H__

#include "geometry/vec.hpp"

namespace PhotonMapping {
using namespace NRenderer;
using namespace std;

class Photon {
   public:
    Vec3 position;
    Vec3 direction;
    Vec3 power;
    int axis = -1;

    Photon() {}
    Photon(Vec3 pos, Vec3 dir, Vec3 power)
        : position(pos), direction(dir), power(power) {}
};
}  // namespace PhotonMapping

#endif
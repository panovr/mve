#ifndef DEFINES_H
#define DEFINES_H

#include <vector>
#include <set>

#include "math/vector.h"

#define MVS_NAMESPACE_BEGIN namespace mvs {
#define MVS_NAMESPACE_END }

/** Multi-View Stereo implementation of [Goesele '07, ICCV]. */
MVS_NAMESPACE_BEGIN

typedef std::set< std::size_t > IndexSet;
typedef std::vector< math::Vec3f > Samples;
typedef std::vector< math::Vec2f > PixelCoords;

const float pi = 3.141592653589793f;

template<typename T>
inline const T sqr(const T& a)
{
    return (a)*(a);
}

MVS_NAMESPACE_END

#endif

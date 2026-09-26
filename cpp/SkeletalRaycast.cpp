#include "TenantTypes.h"
#include <immintrin.h>
#include <cmath>
#include <algorithm>

namespace PhysicsEngine {

    bool RayOBBIntersectionAVX2(
        const Vector3& ray_origin,
        const Vector3& ray_dir,
        const Vector3& aabb_min,
        const Vector3& aabb_max,
        float& out_distance
    ) {
        alignas(32) float orig[4] = { ray_origin.x, ray_origin.y, ray_origin.z, 0.0f };
        alignas(32) float dir[4]  = { ray_dir.x,    ray_dir.y,    ray_dir.z,    1.0f };
        alignas(32) float vmin[4] = { aabb_min.x,   aabb_min.y,   aabb_min.z,   0.0f };
        alignas(32) float vmax[4] = { aabb_max.x,   aabb_max.y,   aabb_max.z,   0.0f };

        __m128 m_orig = _mm_load_ps(orig);
        __m128 m_dir  = _mm_load_ps(dir);
        __m128 m_min  = _mm_load_ps(vmin);
        __m128 m_max  = _mm_load_ps(vmax);

        __m128 inv_dir = _mm_div_ps(_mm_set1_ps(1.0f), m_dir);

        __m128 t1 = _mm_mul_ps(_mm_sub_ps(m_min, m_orig), inv_dir);
        __m128 t2 = _mm_mul_ps(_mm_sub_ps(m_max, m_orig), inv_dir);

        __m128 tmin = _mm_min_ps(t1, t2);
        __m128 tmax = _mm_max_ps(t1, t2);

        alignas(16) float f_tmin[4];
        alignas(16) float f_tmax[4];

        _mm_store_ps(f_tmin, tmin);
        _mm_store_ps(f_tmax, tmax);

        float t_near = std::fmax(std::fmax(f_tmin[0], f_tmin[1]), f_tmin[2]);
        float t_far  = std::fmin(std::fmin(f_tmax[0], f_tmax[1]), f_tmax[2]);

        if (t_near > t_far || t_far < 0.0f) {
            return false;
        }

        out_distance = t_near;
        return true;
    }

}

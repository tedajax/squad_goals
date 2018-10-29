#include "path.h"

path::path(path_dir dir, f32 width, std::initializer_list<vec2> pts)
 : width(width), dir(dir)
{
    this->points.insert(this->points.end(), pts.begin(), pts.end());
}


vec2 path::nearest(const vec2& p, vec2& direction) {
    vec2 ret;
    f32 shortest = std::numeric_limits<f32>::max();

    for (int i = 0; i < points.size(); ++i) {
        int i0 = i;
        int i1 = (i + 1) % points.size();

        if (dir == path_dir::kCCW) {
            std::swap(i0, i1);
        }

        const vec2& pt0 = points[i0];
        const vec2& pt1 = points[i1];
        vec2 a = p - pt0;
        vec2 b = pt1 - pt0;

        b.normalize();
        vec2 pathPt = pt0 + b * a.dot(b);

        pathPt = vec2::on_segment_or_other(pathPt, pt0, pt1, pt1);

        f32 d = vec2::dist(pathPt, p);
        if (d < shortest) {
            shortest = d;
            ret = pathPt;
            direction = (pt1 - pt0).normalize();
        }
    }

    return ret;
}

f32 path::distance(const vec2& p) {
    vec2 _;
    return (p - nearest(p, _)).len();
}

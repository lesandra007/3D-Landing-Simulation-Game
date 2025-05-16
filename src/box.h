#ifndef _BOX_H_
#define _BOX_H_

#include <assert.h>
#include "vector3.h"
#include "ray.h"

/*
 * Axis-aligned bounding box class, for use with the optimized ray-box
 * intersection test described in:
 *
 *      Amy Williams, Steve Barrus, R. Keith Morley, and Peter Shirley
 *      "An Efficient and Robust Ray-Box Intersection Algorithm"
 *      Journal of graphics tools, 10(1):49-54, 2005
 *
 */

class Box {
  public:
    Box() { }
    Box(const Vector3 &min, const Vector3 &max) {
 //     assert(min < max);
      parameters[0] = min;
      parameters[1] = max;
    }
    // (t0, t1) is the interval for valid hits
    bool intersect(const Ray &, float t0, float t1) const;

    // corners
    Vector3 parameters[2];

	Vector3 min() { return parameters[0]; }
	Vector3 max() { return parameters[1]; }

    void changeParameters(const Vector3& min, const Vector3& max) { 
        parameters[0] = min;
        parameters[1] = max;
    }

	const bool inside(const Vector3 &p) {
		return ((p.x() >= parameters[0].x() && p.x() <= parameters[1].x()) &&
		     	(p.y() >= parameters[0].y() && p.y() <= parameters[1].y()) &&
			    (p.z() >= parameters[0].z() && p.z() <= parameters[1].z()));
	}
	const bool inside(Vector3 *points, int size) {
		bool allInside = true;
		for (int i = 0; i < size; i++) {
			if (!inside(points[i])) allInside = false;
			break;
		}
		return allInside;
	}

	// implement for Homework Project
	//
    bool Box::overlap(const Box& box) {
        // Get min and max points for both boxes
        const Vector3& min1 = parameters[0];
        const Vector3& max1 = parameters[1];
        const Vector3& min2 = box.parameters[0];
        const Vector3& max2 = box.parameters[1];

        // Check for overlap in all three dimensions
        // If there's no overlap in any dimension, the boxes don't intersect
        if (max1.x() < min2.x() || min1.x() > max2.x()) return false;
        if (max1.y() < min2.y() || min1.y() > max2.y()) return false;
        if (max1.z() < min2.z() || min1.z() > max2.z()) return false;

        // If we passed all overlap tests, boxes overlap
        return true;
    }

	Vector3 center() {
		return ((max() - min()) / 2 + min());
	}
};

#endif // _BOX_H_

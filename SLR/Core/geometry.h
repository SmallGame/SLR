//
//  geometry.h
//
//  Created by 渡部 心 on 2015/05/05.
//  Copyright (c) 2015年 渡部 心. All rights reserved.
//

#ifndef SLR_geometry_h
#define SLR_geometry_h

#include "../defines.h"
#include "../references.h"
#include "../BasicTypes/Vector3.h"
#include "../BasicTypes/Vector4.h"
#include "../BasicTypes/Normal3.h"
#include "../BasicTypes/Point3.h"
#include "../BasicTypes/Matrix4x4.h"
#include "../BasicTypes/Quaternion.h"
#include "../BasicTypes/TexCoord2.h"
#include "../BasicTypes/Spectrum.h"

struct Ray {
    static const float Epsilon;
    
    Point3D org;
    Vector3D dir;
    float distMin, distMax;
    float time;
    
    Ray() { };
    Ray(const Point3D &o, const Vector3D &d, float t, float dMin = 0.0f, float dMax = INFINITY) :
    org(o), dir(d), distMin(dMin), distMax(dMax), time(t) { };
};

struct BoundingBox3D {
    enum Axis : uint8_t {
        Axis_X = 0,
        Axis_Y,
        Axis_Z,
    };
    
    Point3D minP, maxP;
    
    BoundingBox3D() {
        minP.x = minP.y = minP.z = INFINITY;
        maxP.x = maxP.y = maxP.z = -INFINITY;
    };
    BoundingBox3D(const Point3D &p) : minP(p), maxP(p) { };
    BoundingBox3D(const Point3D &pmin, const Point3D &pmax) : minP(pmin), maxP(pmax) { };
    
    Point3D centroid() const {
        return (minP + maxP) * 0.5f;
    };
    
    float surfaceArea() const {
        Vector3D d = maxP - minP;
        return 2 * (d.x * d.y + d.y * d.z + d.z * d.x);
    };
    
    Point3D corner(uint32_t c) const {
        SLRAssert(c >= 0 && c < 8, "\"c\" is out of range [0, 8]");
        const size_t offset = sizeof(Point3D);
        return Point3D(*(&minP.x + offset * (c & 0x01)),
                       *(&minP.y + offset * (c & 0x02)),
                       *(&minP.z + offset * (c & 0x04)));
    };
    
    float centerOfAxis(uint8_t axis) const {
        return (minP[axis] + maxP[axis]) * 0.5f;
    };
    
    Axis widestAxis() const {
        Vector3D d = maxP - minP;
        if (d.x > d.y && d.x > d.z)
            return Axis_X;
        else if (d.y > d.z)
            return Axis_Y;
        else
            return Axis_Z;
    };
    
    BoundingBox3D &unify(const Point3D &p) {
        minP = min(minP, p);
        maxP = max(maxP, p);
        return *this;
    };
    
    BoundingBox3D unify(const BoundingBox3D &b) {
        minP = min(minP, b.minP);
        maxP = max(maxP, b.maxP);
        return *this;
    };
    
    bool intersect(const Ray &r) const {
        float dist0 = r.distMin, dist1 = r.distMax;
        Vector3D invRayDir = r.dir.reciprocal();
        Vector3D tNear = (minP - r.org) * invRayDir;
        Vector3D tFar = (maxP - r.org) * invRayDir;
        for (int i = 0; i < 3; ++i) {
            if (tNear[i] > tFar[i])
                std::swap(tNear[i], tFar[i]);
            dist0 = tNear[i] > dist0 ? tNear[i] : dist0;
            dist1 = tFar[i] < dist1 ? tFar[i]  : dist1;
            if (dist0 > dist1)
                return false;
        }
        return true;
    };
};


struct Vertex {
    Point3D position;
    Normal3D normal;
    Tangent3D tangent;
    TexCoord2D texCoord;
    
    Vertex() { };
    Vertex(const Point3D &pos, const Normal3D &norm, const Tangent3D &tang, const TexCoord2D &tc) : position(pos), normal(norm), tangent(tang), texCoord(tc) { };
};


class Surface {
public:
    virtual ~Surface() { };
    
    virtual BoundingBox3D bounds() const = 0;
    virtual bool preTransformed() const = 0;
    virtual bool intersect(const Ray &ray, Intersection* isect) const = 0;
    virtual void getSurfacePoint(const Intersection &isect, SurfacePoint* surfPt) const = 0;
    virtual float area() const = 0;
    virtual void sample(float u0, float u1, SurfacePoint* surfPt, float* areaPDF) const = 0;
    virtual float evaluateAreaPDF(const SurfacePoint& surfPt) const = 0;
};

struct ReferenceFrame {
    Vector3D x, y, z;
    
    Vector3D toLocal(const Vector3D &v) const { return Vector3D(dot(x, v), dot(y, v), dot(z, v)); };
    Vector3D fromLocal(const Vector3D &v) const {
        // assume orthonormal basis
        return Vector3D(dot(Vector3D(x.x, y.x, z.x), v),
                        dot(Vector3D(x.y, y.y, z.y), v),
                        dot(Vector3D(x.z, y.z, z.z), v));
    };
};

struct Intersection {
    float time;
    float dist;
    Point3D p;
    Normal3D gNormal;
    float u, v;
    TexCoord2D texCoord;
    mutable std::stack<const SurfaceObject*> obj;
//    const SurfaceObject* obj;
    
    Intersection() : dist(INFINITY) { };
    
    void getSurfacePoint(SurfacePoint* surfPt);
};

struct SurfacePoint {
    Point3D p;
    bool atInfinity;
    Normal3D gNormal;
    float u, v;
    TexCoord2D texCoord;
    Vector3D texCoord0Dir;
    ReferenceFrame shadingFrame;
    const SingleSurfaceObject* obj;
    
    Vector3D getShadowDirection(const SurfacePoint &shadingPoint, float* dist2);
    bool isEmitting() const;
    Spectrum emittance() const;
    float evaluateAreaPDF() const;
    BSDF* createBSDF(ArenaAllocator &mem) const;
    EDF* createEDF(ArenaAllocator &mem) const;
    
    friend SurfacePoint operator*(const StaticTransform &transform, const SurfacePoint &surfPt);
};

#endif
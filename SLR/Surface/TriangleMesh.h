//
//  TriangleMesh.h
//
//  Created by 渡部 心 on 2015/03/22.
//  Copyright (c) 2015年 渡部 心. All rights reserved.
//

#ifndef __SLR__TriangleMesh__
#define __SLR__TriangleMesh__

#include "../defines.h"
#include "../references.h"
#include "../Core/geometry.h"

class Triangle : public Surface {
    TriangleMeshNode* m_mesh;
    uint64_t m_v[3];
    FloatTexture* m_alphaTex;
public:
    Triangle(TriangleMeshNode* mesh, uint64_t v0, uint64_t v1, uint64_t v2, FloatTexture* aTex) :
    m_mesh(mesh), m_v{v0, v1, v2}, m_alphaTex(aTex) { };
    
    BoundingBox3D bounds() const override;
    bool preTransformed() const override;
    bool intersect(const Ray &ray, Intersection* isect) const override;
    void getSurfacePoint(const Intersection &isect, SurfacePoint* surfPt) const override;
    float area() const override;
    void sample(float u0, float u1, SurfacePoint* surfPt, float* areaPDF) const override;
    float evaluateAreaPDF(const SurfacePoint& surfPt) const override;
};

#endif
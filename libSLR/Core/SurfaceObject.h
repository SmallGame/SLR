//
//  SurfaceObject.h
//
//  Created by 渡部 心 on 2015/07/15.
//  Copyright (c) 2015年 渡部 心. All rights reserved.
//

#ifndef __SLR__SurfaceObject__
#define __SLR__SurfaceObject__

#include "../defines.h"
#include "../references.h"
#include "Object.h"

namespace SLR {
    struct SLR_API SurfaceLightPosSample {
        float uPos[2];
        SurfaceLightPosSample(float up0, float up1) : uPos{up0, up1} { }
    };
    
    struct SLR_API SurfaceLightPosQueryResult : public LightPosQueryResult {
        SurfacePoint surfPt;
        float areaPDF;
        
        InteractionPoint* getInteractionPoint() override { return &surfPt; }
        float spatialPDF() const override { return areaPDF; }
    };
    
    class SLR_API SurfaceLight : public Light {
        mutable FixedStack<const SurfaceObject*> m_hierarchy;
    public:
        SurfaceLight() { }
        SurfaceLight(const FixedStack<const SurfaceObject*> &hierarchy) : m_hierarchy(hierarchy) { }
        SurfaceLight(const SurfaceInteraction &si);
        
        void push(const SurfaceObject* obj) const { m_hierarchy.push(obj); }
        ScopedPop<const SurfaceObject*> scopedPop() const { return ScopedPop<const SurfaceObject*>(m_hierarchy); }
        const SurfaceObject* top() const { return m_hierarchy.top(); }
        
        SampledSpectrum sample(const LightPosQuery &query, const SurfaceLightPosSample &smp, SurfaceLightPosQueryResult* result) const;
        Ray sampleRay(const LightPosQuery &lightPosQuery, const SurfaceLightPosSample &lightPosSample,
                      const EDFQuery &edfQuery, const EDFSample &edfSample,
                      ArenaAllocator &mem,
                      SurfaceLightPosQueryResult* lightPosResult, SampledSpectrum* Le0, EDF** edf,
                      EDFQueryResult* edfResult, SampledSpectrum* Le1) const;
        
        SampledSpectrum sample(const LightPosQuery &query, LightPathSampler &pathSampler, ArenaAllocator &mem, LightPosQueryResult** lpResult) const override;
    };
    
    
    
    class SLR_API SurfaceObject : public Object {
    public:
        virtual bool isEmitting() const = 0;
        virtual float importance() const = 0;
        virtual void selectLight(float u, SurfaceLight* light, float* prob) const = 0;
        virtual float evaluateProb(const SurfaceLight &light) const = 0;
        
        virtual SampledSpectrum sample(const SurfaceLight &light,
                                       const LightPosQuery &query, const SurfaceLightPosSample &smp, SurfaceLightPosQueryResult* result) const {
            SLRAssert(light.top() == this, "Object stored in Intersection does not match this object.");
            ScopedPop<const SurfaceObject*> sp = light.scopedPop();
            return light.top()->sample(light, query, smp, result);
        }
        virtual Ray sampleRay(const SurfaceLight &light,
                              const LightPosQuery &lightPosQuery, const SurfaceLightPosSample &lightPosSample,
                              const EDFQuery &edfQuery, const EDFSample &edfSample,
                              ArenaAllocator &mem,
                              SurfaceLightPosQueryResult* lightPosResult, SampledSpectrum* Le0, EDF** edf,
                              EDFQueryResult* edfResult, SampledSpectrum* Le1) const {
            SLRAssert(light.top() == this, "Object stored in Intersection does not match this object.");
            ScopedPop<const SurfaceObject*> sp = light.scopedPop();
            return light.top()->sampleRay(light, lightPosQuery, lightPosSample, edfQuery, edfSample, mem, lightPosResult, Le0, edf, edfResult, Le1);
        }
        
        virtual float costForIntersect() const = 0;
        virtual bool intersect(Ray &ray, SurfaceInteraction* si) const = 0;
        virtual void getSurfacePoint(const SurfaceInteraction &si, SurfacePoint* surfPt) const { SLRAssert_NotImplemented(); }
        
        bool intersect(Ray &ray, SurfacePoint* surfPt) const;
        bool testVisibility(const SurfacePoint &shdP, const SurfacePoint &lightP, float time) const;
        
        bool contains(const Light* light) const override {
            return ((const SurfaceLight*)light)->top() == this;
        }
        float evaluateProbability(const Light* light) const override {
            return evaluateProb(*(const SurfaceLight*)light);
        }
    };
    
    
    
    class SLR_API SingleSurfaceObject : public SurfaceObject {
    protected:
        const Surface* m_surface;
        const SurfaceMaterial* m_material;
    public:
        SingleSurfaceObject() { }
        SingleSurfaceObject(const Surface* surf, const SurfaceMaterial* mat) : m_surface(surf), m_material(mat) { }
        virtual ~SingleSurfaceObject() { }
        
        //----------------------------------------------------------------
        // Object's methods
        BoundingBox3D bounds() const override { return m_surface->bounds(); }
        BoundingBox3D choppedBounds(BoundingBox3D::Axis chopAxis, float minChopPos, float maxChopPos) const override {
            return m_surface->choppedBounds(chopAxis, minChopPos, maxChopPos);
        }
        void splitBounds(BoundingBox3D::Axis chopAxis, float splitPos, BoundingBox3D* bbox0, BoundingBox3D* bbox1) const override {
            m_surface->splitBounds(chopAxis, splitPos, bbox0, bbox1);
        }
        //----------------------------------------------------------------
        
        //----------------------------------------------------------------
        // SurfaceObject's methods
        bool isEmitting() const override;
        float importance() const override;
        void selectLight(float u, SurfaceLight* light, float* prob) const override;
        float evaluateProb(const SurfaceLight &light) const override;
        
        SampledSpectrum sample(const SurfaceLight &light,
                               const LightPosQuery &query, const SurfaceLightPosSample &smp, SurfaceLightPosQueryResult* result) const override;
        Ray sampleRay(const SurfaceLight &light,
                      const LightPosQuery &lightPosQuery, const SurfaceLightPosSample &lightPosSample,
                      const EDFQuery &edfQuery, const EDFSample &edfSample,
                      ArenaAllocator &mem,
                      SurfaceLightPosQueryResult* lightPosResult, SampledSpectrum* Le0, EDF** edf,
                      EDFQueryResult* edfResult, SampledSpectrum* Le1) const override;
        
        float costForIntersect() const override { return m_surface->costForIntersect(); }
        bool intersect(Ray &ray, SurfaceInteraction* si) const override;
        void getSurfacePoint(const SurfaceInteraction &si, SurfacePoint* surfPt) const override;
        //----------------------------------------------------------------
        
        virtual BSDF* createBSDF(const SurfacePoint &surfPt, const WavelengthSamples &wls, ArenaAllocator &mem) const;
        
        virtual float evaluateAreaPDF(const SurfacePoint& surfPt) const;
        virtual SampledSpectrum emittance(const SurfacePoint& surfPt, const WavelengthSamples &wls) const;
        virtual EDF* createEDF(const SurfacePoint &surfPt, const WavelengthSamples &wls, ArenaAllocator &mem) const;
    };
    
    
    
    class SLR_API BumpSingleSurfaceObject : public SingleSurfaceObject {
        const Normal3DTexture* m_normalMap;
    public:
        BumpSingleSurfaceObject(const Surface* surf, const SurfaceMaterial* mat, const Normal3DTexture* normalMap) :
        SingleSurfaceObject(surf, mat), m_normalMap(normalMap) { }
        
        //----------------------------------------------------------------
        // SurfaceObject's methods
        void getSurfacePoint(const SurfaceInteraction &si, SurfacePoint* surfPt) const override;
        //----------------------------------------------------------------
    };
    
    
    
    class SLR_API InfiniteSphereSurfaceObject : public SingleSurfaceObject {
        const Scene* m_scene;
        const RegularConstantContinuous2D* m_dist;
    public:
        InfiniteSphereSurfaceObject(const Scene* scene, const IBLEmission* emitter);
        ~InfiniteSphereSurfaceObject();
        
        //----------------------------------------------------------------
        // SurfaceObject's methods
        bool isEmitting() const override;
        float importance() const override;
        
        SampledSpectrum sample(const SurfaceLight &light,
                               const LightPosQuery &query, const SurfaceLightPosSample &smp, SurfaceLightPosQueryResult* result) const override;
        Ray sampleRay(const SurfaceLight &light,
                      const LightPosQuery &lightPosQuery, const SurfaceLightPosSample &lightPosSample,
                      const EDFQuery &edfQuery, const EDFSample &edfSample,
                      ArenaAllocator &mem,
                      SurfaceLightPosQueryResult* lightPosResult, SampledSpectrum* Le0, EDF** edf,
                      EDFQueryResult* edfResult, SampledSpectrum* Le1) const override;
        //----------------------------------------------------------------
        
        //----------------------------------------------------------------
        // SingleSurfaceObject's methods
        BSDF* createBSDF(const SurfacePoint &surfPt, const WavelengthSamples &wls, ArenaAllocator &mem) const override;
        
        float evaluateAreaPDF(const SurfacePoint& surfPt) const override;
        //----------------------------------------------------------------
    };
    
    
    
    class SLR_API TransformedSurfaceObject : public SurfaceObject {
        const SurfaceObject* m_surfObj;
        const Transform* m_transform;
        friend class Light;
    public:
        TransformedSurfaceObject(const SurfaceObject* surfObj, const Transform* transform) : m_surfObj(surfObj), m_transform(transform) { }
        
        //----------------------------------------------------------------
        // Object's methods
        BoundingBox3D bounds() const override;
        //----------------------------------------------------------------
        
        //----------------------------------------------------------------
        // SurfaceObject's methods
        bool isEmitting() const override;
        float importance() const override;
        void selectLight(float u, SurfaceLight* light, float* prob) const override;
        float evaluateProb(const SurfaceLight &light) const override;
        
        SampledSpectrum sample(const SurfaceLight &light,
                               const LightPosQuery &query, const SurfaceLightPosSample &smp, SurfaceLightPosQueryResult* result) const override;
        Ray sampleRay(const SurfaceLight &light,
                      const LightPosQuery &lightPosQuery, const SurfaceLightPosSample &lightPosSample,
                      const EDFQuery &edfQuery, const EDFSample &edfSample,
                      ArenaAllocator &mem,
                      SurfaceLightPosQueryResult* lightPosResult, SampledSpectrum* Le0, EDF** edf,
                      EDFQueryResult* edfResult, SampledSpectrum* Le1) const override;
        
        float costForIntersect() const override { return m_surfObj->costForIntersect(); }
        bool intersect(Ray &ray, SurfaceInteraction* si) const override;
        void getSurfacePoint(const SurfaceInteraction &si, SurfacePoint* surfPt) const override;
        //----------------------------------------------------------------
        
        void setTransform(const Transform* t) { m_transform = t; }
    };
    
    
    
    class SLR_API SurfaceObjectAggregate : public SurfaceObject {
        Accelerator* m_accelerator;
        const SurfaceObject** m_lightList;
        RegularConstantDiscrete1D* m_lightDist1D;
        std::map<const SurfaceObject*, uint32_t> m_revMap;
    public:
        SurfaceObjectAggregate(std::vector<SurfaceObject*> &objs);
        ~SurfaceObjectAggregate();
        
        //----------------------------------------------------------------
        // Object's methods
        BoundingBox3D bounds() const override;
        //----------------------------------------------------------------
        
        //----------------------------------------------------------------
        // SurfaceObject's methods
        bool isEmitting() const override;
        float importance() const override;
        void selectLight(float u, SurfaceLight* light, float* prob) const override;
        float evaluateProb(const SurfaceLight &light) const override;
        
        float costForIntersect() const override;
        bool intersect(Ray &ray, SurfaceInteraction* si) const override;
        void getSurfacePoint(const SurfaceInteraction &si, SurfacePoint* surfPt) const override {
            SLRAssert(si.top() == this, "Object stored in Intersection does not match this object.");
            ScopedPop<const SurfaceObject*> sp = si.scopedPop();
            si.getSurfacePoint(surfPt);
        }
        //----------------------------------------------------------------
    };
}

#endif

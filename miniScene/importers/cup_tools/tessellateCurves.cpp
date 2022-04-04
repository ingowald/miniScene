// ======================================================================== //
// Copyright 2019-2020 Ingo Wald                                            //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "tessellateCurves.h"
#include "owl/common/parallel/parallel_for.h"
#include <set>

namespace cup {
  namespace tools {
    
    /* contributed by dave hart .... */
    inline void bspline_to_bezier(
                                  const vec3f& i_bsp0, 
                                  const vec3f& i_bsp1, 
                                  const vec3f& i_bsp2, 
                                  const vec3f& i_bsp3, 
                                  vec3f o_bez[4])
    {
      const float rcp6 = 0.166666f;
      o_bez[0] = ( i_bsp0 + 4.f * i_bsp1 +       i_bsp2          ) * rcp6;
      o_bez[1] = (          4.f * i_bsp1 + 2.f * i_bsp2          ) * rcp6;
      o_bez[2] = (          2.f * i_bsp1 + 4.f * i_bsp2          ) * rcp6;
      o_bez[3] = (                i_bsp1 + 4.f * i_bsp2 + i_bsp3 ) * rcp6;
    }

    /* contributed by dave hart .... */
    inline void bezier_eval( const float t, 
                             const vec3f i_bez[4], 
                             vec3f& o_position,
                             vec3f& o_tangent )
    {
      const float t1 = 1.f - t;

      o_tangent = 3.f * (
                         (i_bez[1] - i_bez[0]) * t1 * t1 +
                         (i_bez[2] - i_bez[1]) * 2.f * t * t1 +
                         (i_bez[3] - i_bez[2]) * t * t);

      o_position = 
        i_bez[0] * t1 * t1 * t1 +
        i_bez[1] * 3.f * t * t1 * t1 +
        i_bez[2] * 3.f * t * t * t1 +
        i_bez[3] * t * t * t;
    }

    /* contributed by dave hart .... */
    pbrt::TriangleMesh::SP tessellateCurve(pbrt::Curve::SP curve)
    {
      assert(curve);
    
      pbrt::TriangleMesh::SP mesh = std::make_shared<pbrt::TriangleMesh>();
      mesh->material = curve->material;

      const affine3f xfm = (const affine3f&)curve->transform;
      const vec3f P0 = (const vec3f&)curve->P.front();
      const vec3f P1 = (const vec3f&)curve->P.back();

      // -------------------------------------------------------
      // compute a curve (bi-)normal
      // -------------------------------------------------------
      vec3f N = vec3f(0);
      for (int i=2;i<curve->P.size();i++) {
        const vec3f pi0 = (const vec3f&)curve->P[i-2];
        const vec3f pi1 = (const vec3f&)curve->P[i-1];
        const vec3f pi2 = (const vec3f&)curve->P[i-0];
        N += cross (pi2-pi1,pi1-pi0);
      }
      if (N != vec3f(0.f)) {
        N = normalize(N);
      } else {
        const vec3f L = normalize(P1-P0);
        N = vec3f(L.y,L.z,L.x);
      }

      // -------------------------------------------------------
      // determine how many verts & tris we'll have
      // assuming b-splines that do not interpolate the endpoints
      // -------------------------------------------------------
      const int numCurveCtlPts = (int)curve->P.size();
      const int numCurveCtlPtsExceptLast = (int)curve->P.size() - 1;

      const int samplesPerCtlPt = 3; // iw?:1; // dave: 3
      const int numCtlSegments = (int)curve->P.size() - 2 - 1;

      const int numCurveSamplesExceptLast = numCtlSegments * samplesPerCtlPt;
      const int numCurveSamples = numCurveSamplesExceptLast + 1;

      const int numMeshVerts = 2 * numCurveSamples;
      const int numMeshTris = 2 * numCurveSamplesExceptLast;

      mesh->vertex.reserve(numMeshVerts);
      mesh->normal.reserve(numMeshVerts);
      mesh->index.reserve(numMeshTris);

      // pre-transform all points
      std::vector<vec3f> xp;
      xp.resize(numCurveCtlPts);
      for (int i = 0; i < numCurveCtlPts; i++) {
        xp[i] = (const vec3f&)curve->P[i];
        xp[i] = (const vec3f&)curve->P[i];
      }

      // build the mesh
      vec3f B; // binormal
      vec3f bez_P[4]; // bezier control points
      vec3f skelP, skelT; // evaluated curve position & tangent
      std::vector<vec3f>& meshVerts = (std::vector<vec3f>&)mesh->vertex;
      std::vector<vec3f>& meshNorms = (std::vector<vec3f>&)mesh->normal;

      for (int sampleIdx = 0; sampleIdx < numCurveSamples; sampleIdx++) {
        const int ctlSegmentIdx = sampleIdx / samplesPerCtlPt; // round down
        const int ctlSegmentSampleIdx = ctlSegmentIdx * samplesPerCtlPt;
        const int ctlPtIdx = (sampleIdx < numCurveSamples - 1) 
          ? ctlSegmentIdx - 1 + 1
          : ctlSegmentIdx - 1 + 1 - 1;

        // control point indices should be in-bounds for curve->P
        assert( ctlPtIdx >= 0 );
        assert( ctlPtIdx+3 < curve->P.size() );

        // compute the bezier control points of this segment
        bspline_to_bezier( xp[ctlPtIdx], xp[ctlPtIdx+1], xp[ctlPtIdx+2], xp[ctlPtIdx+3], bez_P );

        // get segment & strand parameter
        const float t_segment = (float)(sampleIdx - ctlSegmentSampleIdx) / (float)samplesPerCtlPt;
        assert(t_segment >= 0.f && t_segment <= 1.f);
        const float t_strand = (float)sampleIdx / (float)(numCurveSamples - 1);
        assert(t_strand >= 0.f && t_strand <= 1.f);


        // interpolate width linearly TODO:read width control points & interpolate as cubic
        const float width = t_strand * curve->width1 + (1.f - t_strand) * curve->width0;
        assert(width > 0.f && width < 2.f);

        bezier_eval(t_segment, bez_P, skelP, skelT);

        // get the binormal
        B = normalize(cross(N, skelT));
        assert(length(B) > 0.f);
        // mesh verts are a pair of skel pointf offset by the binormal
        meshVerts.push_back(xfmPoint(xfm, skelP - N * 0.5f*width));
        meshVerts.push_back(xfmPoint(xfm, skelP + N * 0.5f*width));
        meshNorms.push_back(normalize(xfmNormal(xfm, B)));
        meshNorms.push_back(normalize(xfmNormal(xfm, B)));
        assert(meshVerts.size() <= numMeshVerts);
      }

      for (int vertIdx = 0; vertIdx < numMeshVerts-2; vertIdx += 2) {
        assert(vertIdx+3 < meshVerts.size());
        mesh->index.push_back(pbrt::math::vec3i(vertIdx+0, vertIdx+1, vertIdx+3));
        mesh->index.push_back(pbrt::math::vec3i(vertIdx+0, vertIdx+2, vertIdx+3));
      }
      assert(mesh->index.size() == numMeshTris);

      return mesh;
    }
  
    /*! a pre-processing pass that tesselates all curve geometries in a
      pbrt::Scene. This assumes that the scene is already a single-level
      isntancing scene */
    void tessellateAllCurvesIn(pbrt::Scene::SP scene)
    {
      // ------------------------------------------------------------------
      // *find* all curves in the model
      // ------------------------------------------------------------------
      std::set<pbrt::Object::SP> allObjects;
      for (auto inst : scene->world->instances)
        if (inst && inst->object)
          allObjects.insert(inst->object);
      
      std::vector<pbrt::Curve::SP>   allCurves;
      std::vector<pbrt::Shape::SP *> originalShapePointers;
      for (auto object : allObjects) 
        for (auto &shape : object->shapes)
          if (pbrt::Curve::SP curve = shape->as<pbrt::Curve>()) {
            allCurves.push_back(curve);
            originalShapePointers.push_back(&shape);
          }
      
      std::cout << "#cup.tools: tessellating "
                << allCurves.size() << " curves..." << std::endl;

      parallel_for(allCurves.size(),
                   [&](size_t curveID){
                     pbrt::Curve::SP        curve = allCurves[curveID];
                     pbrt::TriangleMesh::SP mesh  = tessellateCurve(curve);
                     *originalShapePointers[curveID] = mesh;
                   },1024);
      
      std::cout << "#cup.tools: done tessellating curves..." << std::endl;
    }

  } // ::cup::tools
} // ::cup


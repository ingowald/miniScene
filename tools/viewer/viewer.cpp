// ======================================================================== //
// Copyright 2018-2022 Ingo Wald                                            //
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

#include "samples/common/owlViewer/OWLViewer.h"
#include "miniScene/Scene.h"
#include <owl/owl.h>
#include "deviceCode.h"

extern "C" char deviceCode_ptx[];

namespace miniViewer {
    using namespace mini;
    using namespace owl;
    using namespace owl::common;

    struct {
        int spp = 1; //4;
        // int shadeMode = SHADE_TOTALLY_AWESOME;
        struct {
            vec3f vp = vec3f(0.f);
            vec3f vu = vec3f(0.f);
            vec3f vi = vec3f(0.f);
            float fovy = 70;
        } camera;
        vec2i windowSize = vec2i(0, 0);
    } cmdline;

    void usage(const std::string& msg)
    {
        if (msg != "") std::cerr << "Error: " << msg << std::endl << std::endl;
        std::cout << "Usage: ./miniViewer <inputfile.brx>" << std::endl;
        exit(msg != "");
    }

    struct Viewer : public owl::viewer::OWLViewer
    {
        typedef owl::viewer::OWLViewer inherited;

        Viewer(Scene::SP scene)
            : scene(scene)
        {
            createContext();
            createModule();
            createMeshGT();
            createWorld();
            createRayGen();
            createMiss();
            createLPs();
            owlBuildPrograms(owl);
            owlBuildPipeline(owl);
            owlBuildSBT(owl);
        }

        OWLGeom createMesh(Mesh::SP mesh)
        {
            OWLGeom geom = owlGeomCreate(owl, meshGT);
            OWLBuffer indexBuffer
                = owlDeviceBufferCreate(owl, OWL_INT3,
                    mesh->indices.size(),
                    mesh->indices.data());
            OWLBuffer vertexBuffer
                = owlDeviceBufferCreate(owl, OWL_FLOAT3,
                    mesh->vertices.size(),
                    mesh->vertices.data());
            OWLBuffer normalBuffer
                = mesh->normals.empty()
                ? 0
                : owlDeviceBufferCreate(owl, OWL_FLOAT3,
                    mesh->normals.size(),
                    mesh->normals.data());
            OWLBuffer texcoordBuffer
                = mesh->texcoords.empty()
                ? 0
                : owlDeviceBufferCreate(owl, OWL_FLOAT2,
                    mesh->texcoords.size(),
                    mesh->texcoords.data());

            owlTrianglesSetVertices(geom, vertexBuffer,
                mesh->vertices.size(), sizeof(vec3f), 0);
            owlTrianglesSetIndices(geom, indexBuffer,
                mesh->indices.size(), sizeof(vec3i), 0);
            return geom;
        }

        OWLGroup createObject(Object::SP object)
        {
            OWLGroup group = createdObjects[object];
            if (!group) {
                std::vector<OWLGeom> geoms;
                for (auto mesh : object->meshes)
                    geoms.push_back(createMesh(mesh));

                group = owlTrianglesGeomGroupCreate
                (owl, geoms.size(), geoms.data()
                    , OPTIX_BUILD_FLAG_ALLOW_RANDOM_VERTEX_ACCESS | OPTIX_BUILD_FLAG_ALLOW_COMPACTION
                );
                owlGroupBuildAccel(group);
                createdObjects[object] = group;
            }
            return group;
        }

        void createWorld()
        {
            std::vector<OWLGroup> instanceGroups;
            std::vector<affine3f> instanceXfms;

            for (auto inst : scene->instances) {
                if (!inst) continue;
                if (!inst->object) continue;

                instanceGroups.push_back(createObject(inst->object));
                instanceXfms.push_back(inst->xfm);
            }
            world = owlInstanceGroupCreate(owl,
                instanceGroups.size(),
                instanceGroups.data(),
                nullptr,
                (float*)instanceXfms.data());
            owlGroupBuildAccel(world);
        }

        void createMeshGT()
        {
            OWLVarDecl vars[]
                = {
                   { "vertices", OWL_BUFPTR, OWL_OFFSETOF(device::Mesh,vertices) },
                   { /* sentinel */ nullptr }
            };
            meshGT = owlGeomTypeCreate(owl, OWL_GEOM_TRIANGLES, sizeof(int), vars, -1);
            owlGeomTypeSetClosestHit(meshGT, 0, defaultModule, "MeshCH");
        }
        void createContext()
        {
            owl = owlContextCreate(0, 0);
        }
        void createModule()
        {
            defaultModule = owlModuleCreate(owl, deviceCode_ptx);
        }
        void createMiss()
        {
            missProg = owlMissProgCreate(owl, defaultModule, "miss", 0, nullptr, 0);
        }
        void createRayGen()
        {
            rayGen = owlRayGenCreate(owl, defaultModule, "renderFrame",
                0, nullptr, 0);
        }
        void createLPs()
        {
            OWLVarDecl lpVars[]
                = {
                // -------------------------------------------------------
                // common
                // -------------------------------------------------------
                { "world",  OWL_GROUP, OWL_OFFSETOF(device::LaunchParams,world) },
                { "camera", OWL_USER_TYPE(device::Camera), OWL_OFFSETOF(device::LaunchParams,camera) },
                { "fbSize", OWL_INT2, OWL_OFFSETOF(device::LaunchParams,fbSize) },
                { "fbPointer",  OWL_RAW_POINTER, OWL_OFFSETOF(device::LaunchParams,fbPointer) },
                // sentinel:
                { nullptr }
            };

            lp = owlParamsCreate(owl, sizeof(device::LaunchParams),
                lpVars, -1);
        }

        // /*! this function gets called whenever the viewer widget changes camera settings */
        virtual void cameraChanged() override
        {
            inherited::cameraChanged();
            auto& camera = inherited::getCamera();

            const vec3f from = camera.position;
            const vec3f at = camera.getPOI();
            const vec3f up = camera.upVector;
            const float fovy = camera.fovyInDegrees;
            device::Camera devCamera(getWindowSize(),
                from, at, up, fovy);
            owlParamsSetRaw(lp, "camera", &devCamera);
        }

        /*! window notifies us that we got resized */
        virtual void resize(const vec2i& newSize) override
        {
            this->fbSize = newSize;
            // ... tell parent to resize (also resizes the pbo in the window)
            inherited::resize(newSize);

            // // ... and finally: update the camera's aspect
            // setAspect(newSize.x/float(newSize.y));

            cameraChanged();
        }
        /*! gets called whenever the viewer needs us to re-render out widget */
        virtual void render() override
        {
            if (fbSize.x < 0) return;

            static double t_last = -1;

            owlParamsSetGroup(lp, "world", world);
            owlParamsSet2i(lp, "fbSize", fbSize.x, fbSize.y);
            owlParamsSetPointer(lp, "fbPointer", fbPointer);

            owlLaunch2D(rayGen, fbSize.x, fbSize.y, lp);

            double t_now = getCurrentTime();
            static double avg_t = 0.;
            if (t_last >= 0)
                avg_t = 0.8 * avg_t + 0.2 * (t_now - t_last);

            if (1) {
                //        std::cout << "fps : " << (1.f/avg_t) << std::endl;
                char title[1000];
                sprintf(title, "miniViewer - %.2f FPS", (1.f / avg_t));
                glfwSetWindowTitle(this->handle, title);
            }
            t_last = t_now;
        }

        OWLContext  owl;
        OWLModule   defaultModule;
        OWLGeomType meshGT;
        OWLGroup    world;
        OWLParams   lp;
        OWLMissProg missProg;
        OWLRayGen   rayGen;
        std::map<Object::SP, OWLGroup> createdObjects;
        Scene::SP scene;
        vec2i fbSize;
    };

} // ::mini

using namespace miniViewer;

extern "C" int main(int argc, char** argv)
{
    try {
        std::string inFileName = "";
        for (int i = 1; i < argc; i++) {
            const std::string arg = argv[i];
            if (arg[0] != '-') {
                inFileName = arg;
            }
            else if (arg == "-fovy") {
                cmdline.camera.fovy = (float)std::atof(argv[++i]);
            }
            else if (arg == "--camera") {
                cmdline.camera.vp.x = (float)std::atof(argv[++i]);
                cmdline.camera.vp.y = (float)std::atof(argv[++i]);
                cmdline.camera.vp.z = (float)std::atof(argv[++i]);
                cmdline.camera.vi.x = (float)std::atof(argv[++i]);
                cmdline.camera.vi.y = (float)std::atof(argv[++i]);
                cmdline.camera.vi.z = (float)std::atof(argv[++i]);
                cmdline.camera.vu.x = (float)std::atof(argv[++i]);
                cmdline.camera.vu.y = (float)std::atof(argv[++i]);
                cmdline.camera.vu.z = (float)std::atof(argv[++i]);
            }
            else if (arg == "-win" || arg == "--size") {
                cmdline.windowSize.x = std::atoi(argv[++i]);
                cmdline.windowSize.y = std::atoi(argv[++i]);
            }
            else if (arg == "-spp") {
                cmdline.spp = std::stoi(argv[++i]);
            }
            // else if (arg == "-sm" || arg == "--shade-mode") {
            //   cmdline.shadeMode = std::stoi(argv[++i]);
            // }
            else
                usage("unknown cmdline arg '" + arg + "'");
        }

        vec2i windowSize = cmdline.windowSize;
        if (windowSize == vec2i(0)) {
            windowSize =
                vec2i(1024);
        }

        Scene::SP scene = Scene::load(inFileName);

        Viewer viewer(scene);
        box3f sceneBounds;
        sceneBounds = scene->getBounds();
        viewer.enableFlyMode();
        viewer.enableInspectMode();

        if (cmdline.camera.vu != vec3f(0.f)) {
            std::cout << "Camera from command line!"
                << std::endl;
            viewer.setCameraOrientation(/*origin   */cmdline.camera.vp,
                /*lookat   */cmdline.camera.vi,
                /*up-vector*/cmdline.camera.vu,
                /*fovy(deg)*/cmdline.camera.fovy);
        }
        else {
            std::cout << "No camera in model, nor on command line - generating from bounds ...."
                << std::endl;
            viewer.setCameraOrientation(/*origin   */
                sceneBounds.center()
                + vec3f(-.3f, .7f, +1.f) * sceneBounds.span(),
                /*lookat   */sceneBounds.center(),
                /*up-vector*/vec3f(0.f, 1.f, 0.f),
                /*fovy(deg)*/cmdline.camera.fovy);
            viewer.setWorldScale(.1f * length(sceneBounds.span()));
            viewer.showAndRun();//owl::viewer::GlutWindow::run(viewer);
        }
    }
    catch (std::exception& e) {
        std::cout << "Fatal runtime error: " << e.what() << std::endl;
        exit(1);
    }
}

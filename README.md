# miniScene

`miniScene` is a mini-app like scene graph with binary reader/writer,
which was mainly developed as an easy way of reading various
highly instanced models (like Disney's Moana, or the PBRT landscape)
for doing some ray tracing research on. 

## QuickStart (for the most likely use case)

MiniScene has a simple
material model, one level of instances, and objects with one or more
triangle meshes. Comes with several importers/converters, but can be
built with only minimal dependencies. Though the miniScene project
also comes with various different tools for importing or manipulating
model files, the *typical* way you *probably* want to use it is as follows:

a) as a cmake submodule to your own project (this will build a libmini.a, 
   and a `miniScene` target that you can then link to.
   In particular, in this importer-only mode it will have *very* little dependencies.

    add_subdirectory(submodules/miniScene EXCLUDE_FROM_ALL)

b) in your app's CUDA or C++ code, as a binary reader (or writer) for binary `.mini`-files (e.g., 
   using a mini-verison of Moana from here.

    #include <miniScene/Scene.h>
    
    ...
    
    mini::Scene scene = Scene::load(fileName);
    for (auto inst : scene->instances)
        doSomeThingWith(inst->xfm, inst->object);
    ...

Two models you probably *want* to play with are here:

- the Disney Moana Island model (in mini format): https://drive.google.com/file/d/1dbx9iKCYmpJjf_h7EYpl1KH-kAxOWwyd/view?usp=share_link

- the "PBRT Landscape" model (in mini format): https://drive.google.com/file/d/1dvmJMUgbTgPIBmK1JW5gweGJh7hYH770/view?usp=share_link

## Overview of a `mini::Scene` Hierarchy 

Each mini-scene has its geometry content organized in three "layers": at the 
innermost layer are triangle meshes (`mini::Mesh`), that have
the usual entities like vertices (as a `std::vector<vec3f>`), vertex
normals, and texture coordinates; each mesh also has a `mini::Material`
that uses a simple material model and supports both color and alpha textures

Meshes get grouped into logical `mini::Object`s, with each object being a `std::vector` of one or more meshes.
Objects can then get *instantiated* using `mini::Instance`s, with each instance consisting
of a reference to an object, plus an affine transform.

Finally, a `mini::Scene` consists of a list of such instances, plus some "global"
stuff like list of point or quad light sources, env map, etc.

All entities in miniScene are references through `std::shared_pointer`s, commonly using
the shortcut of `Somthing::SP` for the longer-form `std::shared_ptr<Something>` (e.g., 
a `Scene::SP` is a shared-pointer pointing to a `mini::Scene` object). All data
in miniScene (except for "bulk" data like texels in a texture, or vertex arrays in meshes) are
referenced through such shared-pointers, typicaly organized in `std::vector`s, which makes
it very easy to eventually process such a `mini::Scene` using STL mechanisms.

E.g., printing a list of all instances:

    for (auto inst : scene->instances)
        std::cout << "instance w/ transform " << inst->transform 
	          << " and " << inst->object->meshes.size() << " meshes" 
		  << std::endl;

Or, creating a list of all "unique" objects and materials used in the model:

    std::set<Object::SP> allObjects;
    for (auto inst : scene->instances)
       allObjects.insert(inst->object);
    std::set<Material::SP> allMaterials;
    for (auto obj : allObjects)
       for (auto mesh : obj->meshes)
          allMeshes.insert(mesh->material);

A good way of getting an idea of how to use miniScene - to load, iterate over, 
or modify `mini::Scene` objects - is to look at the various cmdline tools in the `apps/`
folder; probably starting with the `miniInfo` tool that collects and prints some basic
info on a mini scene file.

# Various miniScene use Cases 

MiniScene is primarily a library for easly loading `.mini`-formatted
models into renderers; but it also comes with various tools and
importers. The main 'scene' library and `.mini`-reader intentionally
come with very few dependencies, and can also be built stand-alone
without any additional dependencies, without a CUDA install,
etc. However, miniScene also comes with a set of cmdline-tools to
import from other formats, in which case miniScene requires to be
cloned and built with various submodules. The following describes some
of the key usage scenarios:

## I only want to load .mini models, and am not using OWL, do not have CUDA, etc.

Use miniScene as a submodule (say, add it to `<projectRoot>/submodules/miniScene`),
then use in your CMake scripts as

    add_subdirectory(submodules/miniScene EXCLUDE_FROM_ALL)
	
... then link your library or executables to `miniScene`. In your C++
files simply do `mini::Scene::SP scene = mini::Scene::load(...)`, and
done. You do not need CUDA, nor OWL, nor do you need to recursively
clone any of the miniScene submodules.

## I want to load .mini models into my renderer, which *also* uses OWL

If your renderer also uses OWL, then miniScene can simple "re-use" the
OWL module that your renderer already uses. To do this: Make sure your
renderer uses *both* OWL *and* miniScene as submodules, then in your
cmake scripts include the OWL submodules *first*, and the miniScene
submodule *after* the OWL one:

    # include OWL *first*:
	add_subdirectory(submodules/OWL EXCLUDE_FROM_ALL) 
	# include miniScene *after* OWL so it can pick up the target
    add_subdirectory(submodules/miniScene EXCLUDE_FROM_ALL) 

Now if you build your renderer the `miniScene::Scene`'s you load will
automatically use the same `owl::vec3f` classes as OWL will.

## I want to build all of miniScene, including the simple OWL debug viewer

To run the viewer you need to build miniScene with OWL support. You need
to clone miniScene with submodules (`git clone --recursive https://...`).
Then build the project, done. 

This uses OWL, so you need to have CUDA and OptiX installed on your
machine (as described in the README.md that comes with OWL).

## I want to build all of miniScene including advanced importers

To build the importers that can read the Moana PBRT model with PTex
textures etc, you need to clone with submodules (see previous
subsection); then in the cmake dialog enable the
`MINI_BUILD_ADVANCED_IMPORTERS` flag (or configure with `mkdir build;
cd build; cmake .. -DMINI_BUILD_ADVANCED_IMPORTERS=ON`)


# Importing Scenes - Examples

"Importing" for this library means that there's a tool(-chain)
involved in *converting* from another scene/model format to our mini
format. 

Some common/useful examples:

## PBRT v3 landscape model

This uses the `pbrtParser` submodule to first convert from `.pbrt` to
that pbrtParser's pre-parsed binary-pbrt `.pbf` format, then converts
from that to mini:
``` bash
	./pbrt2pdf -o /tmp/landscape.pbf ~/models/pbrt-v3-scenes/landscape/view-0.pbrt
	./pbf2mini -o landscape.mini /tmp/landscape.pbf -t ~/models/pbrt-v3-scenes/landscape
	./miniInfo landscape.mini
```
- Make sure to pass the `-t <texturepath>` during the pbf2mini conversion; pbf only stores the relative filename, and without that textutes cannot be included.

After that you should see this

``` bash
#miniInfo: scene loaded.
----
num instances		:   30.03K	(30035)
num objects		:   29
----
num *unique* meshes	:   370
num *unique* triangles	:   27.78M	(27783147)
num *unique* vertices	:   24.56M	(24558785)
----
num *actual* meshes	:   407.69K	(407691)
num *actual* triangles	:   4.35G	(4349699001)
num *actual* vertices	:   3.25G	(3253912887)
----
num textures		:   156
 - num *ptex* textures	:   0
 - num *image* textures	:   157
total size of textures	:   498.06M	(498064452)
 - #bytes in ptex	:   0
 - #byte in texels	:   498.06M	(498064452)
num materials		:   369
num quad lights		:   0
num dir lights		:   1
has env-map light?	: yes, with 512x256 texels
```

## Moana Island, including baking of PTex Textures

One of the reasons I developed this library was that I needed to be
able to parse certain PBRT-formated files --- in particular, the PBRT
`landscape` and Disney `island` (aka `Moana`) model --- into my own
renderers. In particular for `island`, that also required deadling
with the PTex textures that the model comes with, but which aren't
easily supported on GPUs.  To handle those, this library contains a
ptex 'baking' tool that bakes these textures into a texture (a texture
atlas, to be exact) such that every patch in the input mesh gets NxN
bilinearly interpolated texels assigned to it (with N being a cmdline
argumnet).

To do this conversion and baking step:

- first, download moana, using the base and PBRT packages, as well as textures

- second, convert island.pbrt to pbf using the `pbrt2pbf` tool that
  comes with this library. This is a "binary" version of that model in
  the `pbrtParser` format (but still contains many unparsed things
  like ptx texture names as plain filenames)

```
./pbrt2pbf ~/models/island/pbrt/mountains.pbrt -o /tmp/mountain.pbf
```

- third, convert the file to 'mini', with *embedding* (but not yet *baked* ptex). In this
version of the model all the ptex binary data is directly embedded in the mini file, but
still in ptex format

```
./pbf2mini /tmp/mountain.pbf -o /tmp/mountain-embedded.mini -t ~/models/island/textures/
```

- finally, use the mini ptex baking tool to bake the ptex into regular NxN per patch textures. This assumes that the meshes with ptex on them actually use two triangles per quad - as is the case for the disney island model, but maybe not for all ptex based models. This baking step will bake the ptex into regular textures, and add appropriate texture coordinates to the triangle mesh(es) that use those textures. Note this will increase the size of the model since the vertices with baked texture atlas coordinates will no longer be sharable across neighboring triangles.

```
./miniBakePtex /tmp/mountain-embedded.mini --res 16 -o mountain.mini
```
	 
 This final `mountain.mini` file should be a model that contains only triangle meshes with 'plain' textures on them. E.g.:
 
```
 wald@vk:~/Projects/miniScene/bin$ ./miniInfo ./mountain.mini 
loading mini file from ./mountain.mini
#miniInfo: scene loaded.
----
num instances		:   1.14M	(1135547)
num objects		:   36
----
num *unique* meshes	:   2.37K	(2368)
num *unique* triangles	:   14.38M	(14380782)
num *unique* vertices	:   28.76M	(28761564)
----
num *actual* meshes	:   2.36M	(2356231)
num *actual* triangles	:   13.54G	(13537454354)
num *actual* vertices	:   27.07G	(27074908708)
----
num textures		:   8
 - num *ptex* textures	:   0
 - num *image* textures	:   9
total size of textures	:   110.10M	(110100480)
 - #bytes in ptex	:   0
 - #byte in texels	:   110.10M	(110100480)
num materials		:   30
num quad lights		:   0
num dir lights		:   0
has env-map light?	: no

```
(note the main island model does have a env map, and is much larger; this is only the mountain
geometry---I was running this on my laptop)


## Stanford Model Repository: Atlas Model

The stanford model repo contains various laser-scanned status of
various sizes; each in ply format, but some split across multiple
files. The "atlas" model is one of the larger ones, and comes in 12
files that require some stitching.

Assuming you have the following directory downloaded from that archive:
``` bash
wald@envy:~/Projects/miniScene/bin$ ll /space/atlas/
-rw-r--r-- 1 wald wald 308k Apr 15 13:16 atlas_qtrmm_10_11.matches
-rw-r--r-- 1 wald wald 446M Apr 15 13:15 atlas_qtrmm_10.ply.gz
-rw-r--r-- 1 wald wald 271k Apr 15 13:16 atlas_qtrmm_11_12.matches
-rw-r--r-- 1 wald wald 420M Apr 15 13:15 atlas_qtrmm_11.ply.gz
-rw-r--r-- 1 wald wald 289k Apr 15 13:15 atlas_qtrmm_1_2.matches
-rw-r--r-- 1 wald wald 241M Apr 15 13:16 atlas_qtrmm_12.ply.gz
(and a few more of that)
```
...then the import path for that is as follows:

``` bash
gunzip /space/atlas/*.ply.gz
./ply2mini /space/atlas/atlas_qtrmm -o /space/atlas.mini --stanford-stitch 12
```
Notes:
- the `--stanford-stitch 12` tells the ply reader that there's 12 individual files that require some stitching using the `.matches` files that come with some of these models
- this model is fairly large - you may want to run that on a machine with quite a bit of memory and swap space.
- the outcome of this should look like this
``` bash
./miniInfo /space/atlas.mini 
loading mini file from /space/atlas.mini
#miniInfo: scene loaded.
----
num instances	    	:   1
num objects     		:   1
----
num *unique* meshes	:   12
num *unique* triangles	:   507.51M	(507512682)
num *unique* vertices	:   255.04M	(255035497)
----
num *actual* meshes	:   12
num *actual* triangles	:   507.51M	(507512682)
num *actual* vertices	:   255.04M	(255035497)

```



# miniScene

A mini-app like scene graph with binary reader/writer. Has a simple
material model, one level of instances, and objects with one or more
triangle meshes. Comes with several importers/converters, but can be
built with only minimal dependencies.

# Dependencies

Currently this projects builds on top of OWL (for the convenience of
the OWL vector/math library, and for a sample model viewer for
miniScene models that this project contains). This means this project 
currently does depend on:

- OptiX (7.0 or newer)
- CUDA (11.0 or newer)

This project also uses several other libraries that it pulls in via
submodules.


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



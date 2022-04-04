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

# Dealing with the Disney "island" (aka "Moana") Model and Baking Ptex Textures

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

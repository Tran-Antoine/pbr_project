Advanced Computer Graphics — Final project
======================================

## About this repository

### Project report

An extensive report of all the features implemented can be found in `report/`. To read it, clone the repository and open the `final_report.html` file in a browser (just downloading the html file won't work, as it uses images from the report folder)

### Resource files
- `assets` contains all resource files used to render the scenes, including models (.obj), volumes (.vdb), HDRIs (.exr), grammars (.l6) and textures (.png, .jpg, .exr)
- `scenes` contains all scene files used throughout the project. The main scene of interest is `scenes/final/finalscene.xml`
- `renders` contains a bunch of images rendered from the scenes, including some at various testing stages (to keep a record of the progress)

### Code

- `ext` contains all external libraries used as git submodules
- `include` contains all headers of the project
- `src` contains all source files of the project. To list some interesting code fragments:
  - `integrator/volum7.cpp` is the integrator used to render the final scene
  - `bsdf/microfacet.cpp` is an implementation of a microfacet model
  - `collection/octree.cpp` is an octree data-structure for ray intersection
  - `stats/warp.cpp` contains many sampling algorithms, including hierchical sampling for IBL, delta tracking for free-flight sampling, and inverse CDF mapping for various distributions
  - `parser/lsystem.cpp` is an implementation of 3D stochastic LSystems
  - `volume/procedural.cpp` contains the algorithm for volume generation (both the flowers and the clouds)
- `test` contains some demo code for specific features

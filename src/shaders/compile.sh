#!/usr/bin/env bash

GLSL_PATH=glslc

$GLSL_PATH shader.vert -o vert.spv
$GLSL_PATH shader.frag -o frag.spv
$GLSL_PATH shader.comp -o comp.spv
$GLSL_PATH mesh/meshShader.vert -o mesh/vert.spv
$GLSL_PATH mesh/meshShader.frag -o mesh/frag.spv
$GLSL_PATH light/lightShader.vert -o light/vert.spv
$GLSL_PATH light/lightShader.frag -o light/frag.spv
$GLSL_PATH primitive/primitiveShader.vert -o primitive/vert.spv
$GLSL_PATH primitive/primitiveShader.frag -o primitive/frag.spv
$GLSL_PATH stencil/stencil.vert -o stencil/vert.spv
$GLSL_PATH stencil/stencil.frag -o stencil/frag.spv
$GLSL_PATH postProcessing/postprocessing.vert -o postProcessing/vert.spv
$GLSL_PATH postProcessing/postprocessing.frag -o postProcessing/frag.spv
$GLSL_PATH cubemap/cubemap.vert -o cubemap/vert.spv
$GLSL_PATH cubemap/cubemap.frag -o cubemap/frag.spv
$GLSL_PATH cubemapDepth/shader.vert -o cubemapDepth/vert.spv
$GLSL_PATH cubemapDepth/shader.frag -o cubemapDepth/frag.spv
$GLSL_PATH shadowMap/shadowmap.vert -o shadowMap/vert.spv
$GLSL_PATH shadowMap/shadowmap.frag -o shadowMap/frag.spv
$GLSL_PATH shadowMap/primitive/shadowMapPrimitive.frag -o shadowMap/primitive/frag.spv
$GLSL_PATH shadowMap/primitive/shadowMapPrimitive.vert -o shadowMap/primitive/vert.spv

$GLSL_PATH shadowMap/omnidirectional/primitive/shader.frag -o shadowMap/omnidirectional/primitive/frag.spv
$GLSL_PATH shadowMap/omnidirectional/primitive/shader.vert -o shadowMap/omnidirectional/primitive/vert.spv
$GLSL_PATH shadowMap/omnidirectional/primitive/shader.geom -o shadowMap/omnidirectional/primitive/geom.spv

$GLSL_PATH shadowMap/omnidirectional/mesh/shader.frag -o shadowMap/omnidirectional/mesh/frag.spv
$GLSL_PATH shadowMap/omnidirectional/mesh/shader.vert -o shadowMap/omnidirectional/mesh/vert.spv
$GLSL_PATH shadowMap/omnidirectional/mesh/shader.geom -o shadowMap/omnidirectional/mesh/geom.spv

$GLSL_PATH shadowMap/mesh/shadowMapMesh.vert -o shadowMap/mesh/vert.spv
$GLSL_PATH screenSpaceQuad/screenSpaceQuad.frag -o screenSpaceQuad/frag.spv
$GLSL_PATH screenSpaceQuad/screenSpaceQuad.vert -o screenSpaceQuad/vert.spv
$GLSL_PATH grass/grass.vert -o grass/vert.spv
$GLSL_PATH grass/grass.geom -o grass/geom.spv
$GLSL_PATH grass/grass.frag -o grass/frag.spv
$GLSL_PATH basicCube/shader.vert -o basicCube/vert.spv
$GLSL_PATH basicCube/shader.frag -o basicCube/frag.spv
$GLSL_PATH textureMapping/shader.vert -o textureMapping/vert.spv
$GLSL_PATH textureMapping/shader.frag -o textureMapping/frag.spv
$GLSL_PATH phong/pointLight/shader.vert -o phong/pointLight/vert.spv
$GLSL_PATH phong/pointLight/shader.frag -o phong/pointLight/frag.spv
$GLSL_PATH phong/object/shader.vert -o phong/object/vert.spv
$GLSL_PATH phong/object/shader.frag -o phong/object/frag.spv

#!/usr/bin/env bash

GLSL_PATH=glslc

$GLSL_PATH basicCube/shader.vert -o basicCube/vert.spv
$GLSL_PATH basicCube/shader.frag -o basicCube/frag.spv
$GLSL_PATH textureMapping/shader.vert -o textureMapping/vert.spv
$GLSL_PATH textureMapping/shader.frag -o textureMapping/frag.spv
$GLSL_PATH phong/pointLight/shader.vert -o phong/pointLight/vert.spv
$GLSL_PATH phong/pointLight/shader.frag -o phong/pointLight/frag.spv
$GLSL_PATH phong/object/shader.vert -o phong/object/vert.spv
$GLSL_PATH phong/object/shader.frag -o phong/object/frag.spv
$GLSL_PATH directionalLight/shader.vert -o directionalLight/vert.spv
$GLSL_PATH directionalLight/shader.frag -o directionalLight/frag.spv
$GLSL_PATH shadowMapping/shader.vert -o shadowMapping/vert.spv
$GLSL_PATH shadowMapping/shader.frag -o shadowMapping/frag.spv
$GLSL_PATH shadowMapping/debug/shader.vert -o shadowMapping/debug/vert.spv
$GLSL_PATH shadowMapping/debug/shader.frag -o shadowMapping/debug/frag.spv
$GLSL_PATH shadowMapping/depthReadFrag/shader.frag -o shadowMapping/depthReadFrag/frag.spv
$GLSL_PATH shadowMapping/container/shader.frag -o shadowMapping/container/frag.spv
$GLSL_PATH shadowMapping/offscreen/shader.vert -o shadowMapping/offscreen/vert.spv

#!/usr/bin/env bash

GLSL_PATH=glslc

$GLSL_PATH postProcessing/shader.vert -o postProcessing/vert.spv
$GLSL_PATH postProcessing/shader.frag -o postProcessing/frag.spv
$GLSL_PATH sphere/shader.vert -o sphere/vert.spv
$GLSL_PATH sphere/shader.frag -o sphere/frag.spv
$GLSL_PATH object/shader.vert -o object/vert.spv
$GLSL_PATH object/shader.frag -o object/frag.spv
$GLSL_PATH pingPong/shader.vert -o pingPong/vert.spv
$GLSL_PATH pingPong/shader.frag -o pingPong/frag.spv

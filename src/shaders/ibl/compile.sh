#!/usr/bin/env bash

GLSL_PATH=glslc

$GLSL_PATH sphere/shader.vert -o sphere/vert.spv
$GLSL_PATH sphere/shader.frag -o sphere/frag.spv
$GLSL_PATH object/shader.vert -o object/vert.spv
$GLSL_PATH object/shader.frag -o object/frag.spv
$GLSL_PATH pingPong/shader.vert -o pingPong/vert.spv
$GLSL_PATH pingPong/shader.frag -o pingPong/frag.spv
$GLSL_PATH cubemap/shader.vert -o cubemap/vert.spv
$GLSL_PATH cubemap/shader.frag -o cubemap/frag.spv
$GLSL_PATH hdr/shader.vert -o hdr/vert.spv
$GLSL_PATH hdr/shader.frag -o hdr/frag.spv
$GLSL_PATH irradiance/shader.vert -o irradiance/vert.spv
$GLSL_PATH irradiance/shader.frag -o irradiance/frag.spv
$GLSL_PATH prefilter/shader.vert -o prefilter/vert.spv
$GLSL_PATH prefilter/shader.frag -o prefilter/frag.spv
$GLSL_PATH brdfLUT/shader.vert -o brdfLUT/vert.spv
$GLSL_PATH brdfLUT/shader.frag -o brdfLUT/frag.spv

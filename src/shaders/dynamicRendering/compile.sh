#!/usr/bin/env bash

GLSL_PATH=glslc

$GLSL_PATH shader.vert -o vert.spv
$GLSL_PATH shader.frag -o frag.spv

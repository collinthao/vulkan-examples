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
$GLSL_PATH omniDirectionalShadowMapping/cubemapShadowMap/shader.vert -o omniDirectionalShadowMapping/cubemapShadowMap/vert.spv
$GLSL_PATH omniDirectionalShadowMapping/cubemapShadowMap/shader.geom -o omniDirectionalShadowMapping/cubemapShadowMap/geom.spv
$GLSL_PATH omniDirectionalShadowMapping/cubemapShadowMap/shader.frag -o omniDirectionalShadowMapping/cubemapShadowMap/frag.spv
$GLSL_PATH omniDirectionalShadowMapping/light/shader.vert -o omniDirectionalShadowMapping/light/vert.spv
$GLSL_PATH omniDirectionalShadowMapping/light/shader.frag -o omniDirectionalShadowMapping/light/frag.spv
$GLSL_PATH omniDirectionalShadowMapping/offscreen/shader.vert -o omniDirectionalShadowMapping/offscreen/vert.spv
$GLSL_PATH omniDirectionalShadowMapping/offscreen/shader.geom -o omniDirectionalShadowMapping/offscreen/geom.spv
$GLSL_PATH omniDirectionalShadowMapping/offscreen/shader.frag -o omniDirectionalShadowMapping/offscreen/frag.spv
$GLSL_PATH omniDirectionalShadowMapping/container/shader.frag -o omniDirectionalShadowMapping/container/frag.spv
$GLSL_PATH normalMapping/pointLight/shader.vert -o normalMapping/pointLight/vert.spv
$GLSL_PATH normalMapping/pointLight/shader.frag -o normalMapping/pointLight/frag.spv
$GLSL_PATH normalMapping/object/shader.vert -o normalMapping/object/vert.spv
$GLSL_PATH normalMapping/object/shader.frag -o normalMapping/object/frag.spv
$GLSL_PATH parallaxMapping/pointLight/shader.vert -o parallaxMapping/pointLight/vert.spv
$GLSL_PATH parallaxMapping/pointLight/shader.frag -o parallaxMapping/pointLight/frag.spv
$GLSL_PATH parallaxMapping/object/shader.vert -o parallaxMapping/object/vert.spv
$GLSL_PATH parallaxMapping/object/shader.frag -o parallaxMapping/object/frag.spv
$GLSL_PATH hdr/postProcessing/shader.vert -o hdr/postProcessing/vert.spv
$GLSL_PATH hdr/postProcessing/shader.frag -o hdr/postProcessing/frag.spv
$GLSL_PATH hdr/pointLight/shader.vert -o hdr/pointLight/vert.spv
$GLSL_PATH hdr/pointLight/shader.frag -o hdr/pointLight/frag.spv
$GLSL_PATH hdr/object/shader.vert -o hdr/object/vert.spv
$GLSL_PATH hdr/object/shader.frag -o hdr/object/frag.spv

set GLSL_PATH=glslc

%GLSL_PATH% shader.vert -o vert.spv
%GLSL_PATH% shader.frag -o frag.spv
%GLSL_PATH% shader.comp -o comp.spv
%GLSL_PATH% mesh/meshShader.vert -o mesh/vert.spv
%GLSL_PATH% mesh/meshShader.frag -o mesh/frag.spv
%GLSL_PATH% light/lightShader.vert -o light/vert.spv
%GLSL_PATH% light/lightShader.frag -o light/frag.spv
%GLSL_PATH% primitive/primitiveShader.vert -o primitive/vert.spv
%GLSL_PATH% primitive/primitiveShader.frag -o primitive/frag.spv
%GLSL_PATH% stencil/stencil.vert -o stencil/vert.spv
%GLSL_PATH% stencil/stencil.frag -o stencil/frag.spv
%GLSL_PATH% postProcessing/postprocessing.vert -o postProcessing/vert.spv
%GLSL_PATH% postProcessing/postprocessing.frag -o postProcessing/frag.spv
%GLSL_PATH% cubemap/cubemap.vert -o cubemap/vert.spv
%GLSL_PATH% cubemap/cubemap.frag -o cubemap/frag.spv
%GLSL_PATH% cubemapDepth/shader.vert -o cubemapDepth/vert.spv
%GLSL_PATH% cubemapDepth/shader.frag -o cubemapDepth/frag.spv
%GLSL_PATH% shadowMap/shadowmap.vert -o shadowMap/vert.spv
%GLSL_PATH% shadowMap/shadowmap.frag -o shadowMap/frag.spv
%GLSL_PATH% shadowMap/primitive/shadowmapPrimitive.frag -o shadowMap/primitive/frag.spv
%GLSL_PATH% shadowMap/primitive/shadowmapPrimitive.vert -o shadowMap/primitive/vert.spv
%GLSL_PATH% shadowMap/mesh/shadowmapMesh.vert -o shadowMap/mesh/vert.spv
%GLSL_PATH% screenSpaceQuad/screenSpaceQuad.frag -o screenSpaceQuad/frag.spv
%GLSL_PATH% screenSpaceQuad/screenSpaceQuad.vert -o screenSpaceQuad/vert.spv
%GLSL_PATH% grass/grass.vert -o grass/vert.spv
%GLSL_PATH% grass/grass.geom -o grass/geom.spv
%GLSL_PATH% grass/grass.frag -o grass/frag.spv
pause

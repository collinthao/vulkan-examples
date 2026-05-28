set GLSL_PATH=glslc.exe

%GLSL_PATH% shader.vert -o vert.spv
%GLSL_PATH% shader.frag -o frag.spv
%GLSL_PATH% shader.comp -o comp.spv
%GLSL_PATH% mesh/meshShader.vert -o mesh/meshVert.spv
%GLSL_PATH% mesh/meshShader.frag -o mesh/meshFrag.spv
%GLSL_PATH% light/lightShader.vert -o light/lightVert.spv
%GLSL_PATH% light/lightShader.frag -o light/lightFrag.spv
%GLSL_PATH% primitive/primitiveShader.vert -o primitive/primitiveVert.spv
%GLSL_PATH% primitive/primitiveShader.frag -o primitive/primitiveFrag.spv
%GLSL_PATH% stencil/stencil.vert -o stencil/stencilVert.spv
%GLSL_PATH% stencil/stencil.frag -o stencil/stencilFrag.spv
%GLSL_PATH% postProcessing/postprocessing.vert -o postProcessing/postprocessingVert.spv
%GLSL_PATH% postProcessing/postprocessing.frag -o postProcessing/postprocessingFrag.spv
%GLSL_PATH% cubemap/cubemap.vert -o cubemap/cubemapVert.spv
%GLSL_PATH% cubemap/cubemap.frag -o cubemap/cubemapFrag.spv
%GLSL_PATH% shadowMap/shadowmap.vert -o shadowMap/shadowmapVert.spv
%GLSL_PATH% shadowMap/shadowmap.frag -o shadowMap/shadowmapFrag.spv
%GLSL_PATH% shadowMap/primitive/shadowmapPrimitive.frag -o shadowMap/primitive/shadowmapPrimitiveFrag.spv
%GLSL_PATH% shadowMap/primitive/shadowmapPrimitive.vert -o shadowMap/primitive/shadowmapPrimitiveVert.spv
%GLSL_PATH% shadowMap/mesh/shadowmapMesh.vert -o shadowMap/mesh/shadowmapMeshVert.spv
%GLSL_PATH% screenSpaceQuad/screenSpaceQuad.frag -o screenSpaceQuad/screenSpaceQuadFrag.spv
%GLSL_PATH% screenSpaceQuad/screenSpaceQuad.vert -o screenSpaceQuad/screenSpaceQuadVert.spv
%GLSL_PATH% grass/grass.vert -o grass/grassVert.spv
%GLSL_PATH% grass/grass.geom -o grass/grassGeom.spv
%GLSL_PATH% grass/grass.frag -o grass/grassFrag.spv
pause

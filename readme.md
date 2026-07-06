To run, go to the /build directory in your cli of choice and run:
```
cmake CMakeLists.txt ..
```
```
cmake --build .
```

***TODO***
- have shadow map view from light perspective // DONE
- remove shadowmap from primitives and make shadowmap screen space quad instead // DONE
- make light count dynamic. Not doing so can be forgotten and cause unforeseen lighting bugs
- make vulkanRenderer have a dependency injection for VkApp instead of a static implementation. This will allow switching applications seamlessly and not having to write all code in one class // DONE
- Cleanup Shaders and put in separate directories
- Cleanup Pipelines and put in a pipeline manager
- Cleanup files in root directory and put in own separate directories
- Replace batch file with CMAKE
- Use glfwGetCursorPos to allow dependency injection for switching mouse movement function per vulkan App
- Fix shadow mapping scene and grass scene to align with work done in vulkanApp header file

**MINECRAFT TODO**
- add dirtblocks and water
XDG_SESSION_TYPE=x11

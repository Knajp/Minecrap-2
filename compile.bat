%VULKAN_SDK%/Bin/glslc.exe src/shaderSource/shader.vert -o src/bin/vert.spv
%VULKAN_SDK%/Bin/glslc.exe src/shaderSource/shader.frag -o src/bin/frag.spv
%VULKAN_SDK%/Bin/glslc.exe src/shaderSource/occlusion.vert -o src/bin/overt.spv
%VULKAN_SDK%/Bin/glslc.exe src/shaderSource/occlusion.frag -o src/bin/ofrag.spv

pause
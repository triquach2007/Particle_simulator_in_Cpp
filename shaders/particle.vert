#version 450

layout(location = 0) in vec2 position;

layout(set = 1, binding = 0) uniform Camera
{
    float width;
    float height;
};

void main()
{
    // Convert SDL screen coordinates to Vulkan NDC.

    float x = (position.x / width) * 2.0 - 1.0;

    float y = 1.0 - (position.y / height) * 2.0;

    gl_Position = vec4(x, y, 0.0, 1.0);

    gl_PointSize = 4.0;
}
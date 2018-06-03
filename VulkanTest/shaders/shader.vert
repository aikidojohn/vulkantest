#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 fragTexCoord;
layout(location = 1) out vec3 surfaceNormal;
layout(location = 2) out vec3 toLightVector;

out gl_PerVertex {
    vec4 gl_Position;
};

const vec3 lightPosition = vec3(75, 75, 75);

void main() {
    vec4 worldPosition = ubo.view * ubo.model * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * worldPosition;
    fragTexCoord = inTexCoord;

    surfaceNormal = normalize((ubo.view * vec4(inNormal, 0.0)).xyz);
    toLightVector = normalize(lightPosition - worldPosition.xyz);
}
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragTexCoord;
layout(location = 1) in vec3 surfaceNormal;
layout(location = 2) in vec3 toLightVector;

layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2DArray texSampler;

const vec3 lightColor = vec3(1,1,1);

void main() {
    float df = dot(surfaceNormal, toLightVector);
    float brightness = max(df, 0.0);
    vec3 diffuse = brightness * lightColor;
    outColor = texture(texSampler, fragTexCoord);
}
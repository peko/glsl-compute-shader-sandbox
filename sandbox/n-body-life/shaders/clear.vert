#version 460 core

layout (location = 0) in vec3 position;

out gl_PerVertex {
  vec4 gl_Position;
};

void main()
{
   gl_Position = vec4(position.xyz, 1.0);
}
#version 460 core

in vec4 base_color;

layout(location = 0) out vec4 fragColor;

void main() {
  vec2 uv = 2.0 * gl_PointCoord - vec2(1);
  float r = length(uv);
  fragColor =  r < 1.0 ? base_color*(cos(r*3.141)/2.0+0.6) : vec4(0.0) ;
}

// void main() {
//   vec2 uv = 2.0 * gl_PointCoord - vec2(1);
//   float r = length(uv);
//   fragColor = vec4(base_color*(0.5/r*base_color), 0.2);
//   // fragColor = vec4(1.0);
// }

#version 330 core
//==============================================================================
layout(location = 0) out vec4 fragout;
//==============================================================================
uniform vec4 color;
//==============================================================================
void main() {
  fragout = color;
}

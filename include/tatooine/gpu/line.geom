#version 330 core
//------------------------------------------------------------------------------
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;
//------------------------------------------------------------------------------
in vec3  geom_position[2];
in vec3  geom_tangent[2];
in float geom_parameterization[2];
//------------------------------------------------------------------------------
out vec3  frag_position;
out vec3  frag_tangent;
out float frag_parameterization;
out float frag_contour_parameterization;
//------------------------------------------------------------------------------
uniform mat4  projection_matrix;
uniform mat4  modelview_matrix;
uniform float line_width;
//------------------------------------------------------------------------------
void main() {
  vec3 p0mv   = (modelview_matrix * vec4(geom_position[0], 1)).xyz;
  vec3 p1mv   = (modelview_matrix * vec4(geom_position[1], 1)).xyz;
  vec3 tan0mv = (modelview_matrix * vec4(geom_tangent[0], 0)).xyz;
  vec3 tan1mv = (modelview_matrix * vec4(geom_tangent[1], 0)).xyz;
  vec3 y0     = normalize(cross(tan0mv, p0mv));
  vec3 y1     = normalize(cross(tan1mv, p1mv));
  vec3 e0     = line_width * y0;
  vec3 e1     = line_width * y1;

  frag_contour_parameterization = -line_width;
  frag_position                 = p0mv;
  frag_tangent                  = tan0mv;
  frag_parameterization         = geom_parameterization[0];
  gl_Position                   = projection_matrix * vec4(p0mv + e0, 1);
  EmitVertex();

  frag_contour_parameterization = -line_width;
  frag_position                 = p1mv;
  frag_tangent                  = tan1mv;
  frag_parameterization         = geom_parameterization[1];
  gl_Position                   = projection_matrix * vec4(p1mv + e1, 1);
  EmitVertex();

  frag_contour_parameterization = line_width;
  frag_position                 = p0mv;
  frag_tangent                  = tan0mv;
  frag_parameterization         = geom_parameterization[0];
  gl_Position                   = projection_matrix * vec4(p0mv - e0, 1);
  EmitVertex();

  frag_contour_parameterization = line_width;
  frag_position                 = p1mv;
  frag_tangent                  = tan1mv;
  frag_parameterization         = geom_parameterization[1];
  gl_Position                   = projection_matrix * vec4(p1mv - e1, 1);
  EmitVertex();

  EndPrimitive();
}

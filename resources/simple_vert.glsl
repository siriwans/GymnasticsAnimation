#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightPos;
out vec3 lightDir;
out vec3 view;
out vec3 normal;
void main()
{
	gl_Position = P * V * M * vertPos;
   lightDir = lightPos - vec3(M * vertPos).xyz;
   normal = (M * vec4(vertNor, 0.0)).xyz;
   view = -1 * (M * vertPos).xyz;
	//gl_Position = P * V * M * vertPos;
   //fragNor = (M * vec4(vertNor, 0.0)).xyz;
}

#version 330 core
uniform sampler2D Texture0;

in vec2 vTexCoord;
//in float dCo;
float dCo;
out vec4 Outcolor;

in vec3 normal;
in vec3 lightDir;

void main() {
   vec4 texColor0 = texture(Texture0, vTexCoord);
	
   vec3 LD = normalize(lightDir);
   //IN THE CPU ADD GLUNIFORM1I to -1
   dCo = max(0.0, dot(normalize(normal), LD));

   //Outcolor = texColor0 * vec4(diffuse, 1.0); 

   Outcolor = texColor0 * dCo; 
	//Outcolor = vec4(vTexCoord.s, vTexCoord.t, 0, 1);
}


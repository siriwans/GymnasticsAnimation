#version 330 core 
in vec3 normal;
in vec3 lightDir;
in vec3 view;
uniform vec3 viewPos;
//in vec3 fragNor
uniform vec3 MatDif;
uniform vec3 MatAmb;
uniform vec3 MatSpec;
uniform vec3 lightColor;
uniform float shine;
out vec4 color;

void main()
{
   //ambient
   float ambientStrength = 0.1;
   vec3 ambient = ambientStrength * lightColor * MatAmb;
   //color = vec4(ambient, 1.0);

   //diffuse
   vec3 LD = normalize(lightDir);
   vec3 diffuse = MatDif * max(0.0, dot(normalize(normal),LD)) * lightColor;

   //specular
   vec3 viewDir = normalize(view);
   //vec3 reflectDir = normalize(normalize(-lightDir) + 2 * max(dot(normalize(lightDir),
      //normalize(normal)), 0.0) * normalize(normal) );
   vec3 halfDir = normalize(lightDir + viewDir);
   float spec = pow(max(dot(normal, halfDir), 0.0), shine);
   vec3 specular = spec * lightColor * MatSpec;
   
   color = vec4(diffuse, 1.0) + vec4(ambient, 1.0) + vec4(specular, 1.0);
}

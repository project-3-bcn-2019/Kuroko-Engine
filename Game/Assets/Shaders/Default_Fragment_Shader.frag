#version 330

in vec3 FragPos;
in vec4 ourColor;
in vec2 TexCoord;
in vec3 ret_normal;

out vec4 color;

uniform sampler2D ourTexture;
uniform int test;
uniform vec3 lightPos;
uniform vec3 lightColor;

void main()
{
	if(test==1)
	{
		color=texture(ourTexture,TexCoord);
	}
	else 
	{
		float ambientStrength=1.0;
		vec3 ambient= ambientStrength*lightColor;
			
		vec3 norm = normalize(ret_normal);
		vec3 lightDir = normalize(lightPos - FragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = diff * lightColor;

		vec3 result = (ambient + diffuse) * vec3(ourColor.x,ourColor.y,ourColor.z);
		color=vec4(result, 1.0);
	}
}
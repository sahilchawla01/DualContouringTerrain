#version 330 core

struct Material 
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shine;
};

struct Light {
	vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};


uniform Material mat;
uniform Light dirLight;
uniform vec3 objectColor;


in vec3 vPos; 
in vec3 vColor; 
in vec3 normal;
in vec3 FragViewPosition;

out vec4 FragColor;

void main()
{

	//-- Calculate AMBIENT component of Phong --

	float ambientStrength = 0.4f;
	vec3 ambient = dirLight.ambient * ( ambientStrength * mat.ambient);


	// -- Calculate DIFFUSE component of Phong -- 

	float diffuseStrength = 0.5f;
	vec3 finalNormal = normalize(normal);
	vec3 dirLightDirection = normalize(-dirLight.direction);
	//Calculate dot product to get diffuse strength, and get max between 0 and dot product (to eradicate negative strength)
	float diff = max(dot(finalNormal, dirLightDirection), 0.0);
	//Final value for diffuse
	vec3 diffuse =  dirLight.diffuse * (diff * mat.diffuse);


	//-- Calculate SPECULAR component of Phong -- 


	float specularStrength = 0.5f;
	vec3 viewDirection = normalize(-FragViewPosition);
	vec3 reflectDir = reflect(-dirLightDirection, finalNormal);
	//Here, 32 is the "shinyness" value of the object i.e a property of the material 
	float spec = pow(max(dot(viewDirection, reflectDir), 0.0), mat.shine);
	//Finally calculate the specular vector
	vec3 specular =  dirLight.specular * specularStrength * (spec * mat.specular);
	
	//-- Calculate Phong RESULTANT -- 
	vec3 phongResult = (ambient + diffuse + specular) * objectColor;

	//-- Add a little color from Position as well -- 
	//phongResult = phongResult + 0.1 * vPos;
	phongResult = vColor;

	FragColor = vec4(phongResult, 1.0);
}
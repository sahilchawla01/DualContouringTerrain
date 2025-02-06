#version 330 core

struct Material 
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shine;
};

struct Light {
    vec3 viewSpacePosition;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};


uniform Material mat;
uniform Light light;

uniform vec3 objectColor;
//Redundant as camera view position in view space is 0,0,0
uniform vec3 cameraViewPosition;

in vec3 normal;
in vec3 FragViewPosition;

out vec4 FragColor;

void main()
{

	//-- Calculate AMBIENT component of Phong --

	float ambientStrength = 0.1f;
	vec3 ambient = light.ambient * ( ambientStrength * mat.ambient);


	// -- Calculate DIFFUSE component of Phong -- 

	float diffuseStrength = 0.5f;
	vec3 finalNormal = normalize(normal);
	vec3 lightDirection = normalize(light.viewSpacePosition - FragViewPosition);
	//Calculate dot product to get diffuse strength, and get max between 0 and dot product (to eradicate negative strength)
	float diff = max(dot(finalNormal, lightDirection), 0.0);
	//Final value for diffuse
	vec3 diffuse =  light.diffuse * (diff * mat.diffuse);


	//-- Calculate SPECULAR component of Phong -- 


	float specularStrength = 0.5f;
	vec3 viewDirection = normalize(cameraViewPosition - FragViewPosition);
	vec3 reflectDir = reflect(-lightDirection, finalNormal);
	//Here, 32 is the "shinyness" value of the object i.e a property of the material 
	float spec = pow(max(dot(viewDirection, reflectDir), 0.0), mat.shine);
	//Finally calculate the specular vector
	vec3 specular =  light.specular * specularStrength * (spec * mat.specular);
	
	//-- Calculate Phong RESULTANT -- 
	vec3 phongResult = (ambient + diffuse + specular) * objectColor;

	FragColor = vec4(phongResult, 1.0);
}
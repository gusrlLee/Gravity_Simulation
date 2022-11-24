#version 330 core 

in vec3 Normal;
in vec2 TexCoord;
in vec3 FragPos;

out vec4 FragColor;

uniform vec3 viewPos;
uniform vec3 lightPos;

uniform sampler2D water_texture;

void main()
{
    vec3 objectColor = vec3(0.0, 0.5, 1.0);
    // vec3 objectColor = texture(water_texture, TexCoord).rgb;
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    // ambient
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  
        
    vec3 result = (ambient + diffuse + specular) * objectColor;

    float ratio = 1.00 / 1.52;
    vec3 I = normalize(FragPos - viewPos);
    vec3 R = refract(I, normalize(Normal), ratio);
    // FragColor = mix(vec4(texture(water_texture), 1.0), vec4(result, 1.0), 0.10);
    //FragColor = mix(texture(water_texture, TexCoord), vec4(result, 1.0), 0.30);
    FragColor = vec4(result, 0.3);
}

//  FragColor = vec4(0.0, 0.0, 0.9, 0.2);

//    vec3 lightColor = vec3(0.0, 0.0, 1.0);
//    // ambient 
//    float ambientStrength = 0.5;
//    vec3 ambient = ambientStrength * lightColor;
//
//    // diffuse 
//    vec3 lightPos = vec3(0, 30, 0);
//    vec3 norm = normalize(Normal);
//    vec3 lightDir = normalize(lightPos - FragPos);
//    float diff = max(dot(norm, lightDir), 0.0);
//    vec3 diffuse = diff * lightColor;
//
//    // specular 
//    float specularStrength = 0.7;
//    vec3 viewDir = normalize(viewPos - FragPos);
//    vec3 reflectDir = reflect(-lightDir, norm);
//    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
//    vec3 specular = specularStrength * spec * lightColor;
//    
//    vec3 result = (ambient + diffuse + specular) * lightColor;
//    FragColor = vec4(result, 1.0);


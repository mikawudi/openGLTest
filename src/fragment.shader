#version 330 core
in vec2 texCoord;
in vec3 Normal;
in vec3 forgPos;
out vec4 FragColor;
uniform sampler2D ourTexture;
uniform sampler2D refleTexture;
uniform float ambientStrength;
uniform vec3 lightPosition;
uniform vec3 camPos;

void main()
{
	//��������
	vec3 lightDir = normalize(lightPosition - forgPos);
	//��������
	vec3 norm = normalize(Normal);
	//�������
	float diff = max(dot(norm, lightDir), 0.0) * 0.4;

	//���շ�������
	vec3 reflectDir = reflect(-lightDir, norm);
	//������۲�����
	vec3 camDir = normalize(camPos - forgPos);
	//���淴���
	float spec = pow(max(dot(camDir, reflectDir), 0.0), 128);
	spec = texture(refleTexture, texCoord).x * spec;


	vec4 color = texture(ourTexture, texCoord);
	FragColor = color * (ambientStrength + diff + spec);
}
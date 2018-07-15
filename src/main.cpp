#include "../include/glad/glad.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"
#include <glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "../include/glm/glm.hpp"
#include "../include/glm/gtc/matrix_transform.hpp"
#include "../include/glm/gtc/type_ptr.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
std::ostringstream* GetShaderSourceFile(const char* path)
{
	std::ifstream shaderFile(path);
	std::ostringstream* vertexBuf = new std::ostringstream();
	*vertexBuf << shaderFile.rdbuf() << std::endl;
	return vertexBuf;
}

class AbstractShader 
{
public:
	virtual bool Init() = 0;
	AbstractShader(char* const filename);
	virtual unsigned int GetShaderId() { return this->shaderId; }
protected:
	char* sourceFile;
	char* errorData;
	unsigned int shaderId;
};

AbstractShader::AbstractShader(char* const filename) 
{
	this->sourceFile = filename;
	this->errorData = (char*)malloc(sizeof(char) * 50);
}

class VertexShader : public AbstractShader
{
public:
	VertexShader::VertexShader(char* const filename);
	bool VertexShader::Init() override;
};

VertexShader::VertexShader(char* const filename) 
	:AbstractShader(filename) 
{

}

bool VertexShader::Init()
{
	this->shaderId = glCreateShader(GL_VERTEX_SHADER);
	auto sourcecode = GetShaderSourceFile(this->sourceFile);
	auto sourcedata = sourcecode->str();
	auto source_c_str = sourcedata.c_str();
	glShaderSource(this->shaderId, 1, &source_c_str, NULL);
	glCompileShader(this->shaderId);
	int success;
	glGetShaderiv(this->shaderId, GL_COMPILE_STATUS, &success);
	if (!success)
		glGetShaderInfoLog(this->shaderId, 512, NULL, this->errorData);
	return success != 0;
}

class FragmentShader : public AbstractShader
{
public:
	FragmentShader::FragmentShader(char* const filename);
	bool FragmentShader::Init() override;
};

FragmentShader::FragmentShader(char* const filename) 
	: AbstractShader(filename)
{

}

bool FragmentShader::Init() 
{
	this->shaderId = glCreateShader(GL_FRAGMENT_SHADER);
	auto sourcecode = GetShaderSourceFile(this->sourceFile);
	auto sourcedata = sourcecode->str();
	auto source_c_str = sourcedata.c_str();
	glShaderSource(this->shaderId, 1, &source_c_str, NULL);
	glCompileShader(this->shaderId);
	int success;
	glGetShaderiv(this->shaderId, GL_COMPILE_STATUS, &success);
	if (!success)
		glGetShaderInfoLog(this->shaderId, 512, NULL, this->errorData);
	return success != 0;
}
class VertexBufferObject;
class ShaderProgramer;
class VertexAttributeObject
{
public:
	GLuint ID;
	GLuint EBOID = -1;
	VertexAttributeObject();
	bool CreateVertexAttribute(char* const attrName, ShaderProgramer* sp, VertexBufferObject* vbo);
	bool BindElementBufferObject(int datesize, void* data);
	bool UseThisVAO() { glBindVertexArray(this->ID); return true; }
};
VertexAttributeObject::VertexAttributeObject()
{
	glGenVertexArrays(1, &this->ID);
}
class VertexBufferObject
{
public:
	VertexBufferObject(char* const vboname, int datasize, void* data, GLenum type, int typesize, int buffelementsize);
	bool UseThisVBO() 
	{ 
		glBindBuffer(GL_ARRAY_BUFFER, this->ID); 
		return true; 
	}
	GLuint ID;
	char* const vboName;
	int dataSize;
	void* data;
	GLenum type;
	int typeSize;
	int bufferElementSize;
};

VertexBufferObject::VertexBufferObject(char* const vboname, int datasize, void* data, GLenum type, int typesize, int buffelementsize)
	: vboName(vboname),
	dataSize(datasize),
	data(data),
	type(type),
	typeSize(typesize),
	bufferElementSize(buffelementsize)
{
	glGenBuffers(1, &this->ID);
	glBindBuffer(GL_ARRAY_BUFFER, this->ID);
	glBufferData(GL_ARRAY_BUFFER, this->dataSize, this->data, GL_STATIC_DRAW);
}

class ShaderProgramer 
{
public:
	AbstractShader* VertexShaderObj;
	AbstractShader* FragmentShaderObj;
	ShaderProgramer(char* const vertexShaderSourceFile, char* const fragmentShaderSourceFile);
	bool Init();
	void UseThisProgram() { glUseProgram(this->programID); }
	GLint GetUnifLocation(const char* name) { return glGetUniformLocation(this->programID, name); }
	GLint GetAttLocation(const char* name) { return glGetAttribLocation(this->programID, name); }
protected:
	unsigned int programID;
};

ShaderProgramer::ShaderProgramer(char* const vertexShaderSourceFile, char* const fragmentShaderSourceFile)
{
	this->VertexShaderObj = new VertexShader(vertexShaderSourceFile);
	this->FragmentShaderObj = new FragmentShader(fragmentShaderSourceFile);
}

bool ShaderProgramer::Init() 
{
	if (!this->VertexShaderObj->Init()) 
	{
		return false;
	}
	if (!this->FragmentShaderObj->Init())
	{
		return false;
	}
	this->programID = glCreateProgram();
	glAttachShader(this->programID, this->VertexShaderObj->GetShaderId());
	glAttachShader(this->programID, this->FragmentShaderObj->GetShaderId());
	glLinkProgram(this->programID);
	return true;
}

bool VertexAttributeObject::CreateVertexAttribute(char* const attrName, ShaderProgramer* sp, VertexBufferObject* vbo)
{
	if (sp == NULL || vbo == NULL)
		return false;
	sp->UseThisProgram();
	auto attlayout = sp->GetAttLocation(attrName);
	if (attlayout == -1)
		return false;
	this->UseThisVAO();
	vbo->UseThisVBO();
	//强制一个结构一个stridesize和元素size一致(不允许多个attribute公用一个size,所以也就0偏移了)
	glVertexAttribPointer(attlayout, vbo->bufferElementSize, vbo->type, GL_FALSE, vbo->bufferElementSize * vbo->typeSize, (void*)0);
	glEnableVertexAttribArray(attlayout);
	return true;
}

bool VertexAttributeObject::BindElementBufferObject(int datesize, void* data) 
{
	glGenBuffers(1, &this->EBOID);
	this->UseThisVAO();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBOID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, datesize, data, GL_STATIC_DRAW);
	return true;
}

class TexureManager 
{
public:
	static GLuint CreateTexture(char* const path);
	static bool SwitchTexture(GLuint textureID, GLint layout, int textureunitId);
private:
	TexureManager() {}
};

class ICamera
{
public:
	virtual glm::mat4 GetViewModel() = 0;
	glm::vec3 GetCamPosition() { return this->Position; }
protected:
	glm::vec3 WorldUp;
	glm::vec3 Position;
	glm::vec3 Front;
};

class SampleCamera
	: public ICamera
{
private:
	glm::vec3 camPosition;
	glm::vec3 camFront;
	glm::vec3 camUP;
public:
	SampleCamera(glm::vec3 cpos, glm::vec3 cfront, glm::vec3 cup)
		: camPosition(cpos),
		camFront(cfront),
		camUP(cup) {}
	glm::mat4 GetViewModel() override 
	{
		auto s = camPosition;
		//真实的摄像机方向是eye-center, normal(camPosition - (camPosition + camFront)) = normal(-camFront).......center的意思并不是让你直接传入算好的方向...
		auto result = glm::lookAt(camPosition, camPosition + camFront, camUP);
		return result;
	}
	void MoveFront() { camPosition += 0.5f * camFront; }
	void MoveBack() { camPosition -= 0.5f * camFront; }
	void MoveRight() 
	{
		camPosition += glm::normalize(glm::cross(this->camFront, this->camUP)) * 0.5f;
	}
	void MoveLeft()
	{
		camPosition -= glm::normalize(glm::cross(this->camFront, this->camUP)) * 0.5f;
	}
};

class EulerCamera
	: public ICamera
{
public:
	EulerCamera(glm::vec3 pos, glm::vec3 cfront, glm::vec3 wup) 
	{ 
		this->Position = pos;
		this->Front = cfront;
		this->WorldUp = wup;
	}
	glm::mat4 GetViewModel() override;
	void PitchUp() { Pitch += 0.1f; }
	void PitchDown() { Pitch -= 0.1f; }
	void YawLeft() { Yaw -= 0.1f; }
	void YawRight() { Yaw += 0.1f; }

	void MoveFront() { this->Position += this->Front * 0.5f; }
	void MoveBack() { this->Position -= this->Front * 0.5f; }
	void MoveRight() { this->Position += glm::normalize(glm::cross(this->Front, this->WorldUp))*0.5f; }
	void MoveLeft() { this->Position -= glm::normalize(glm::cross(this->Front, this->WorldUp))*0.5f; }
protected:
	///俯仰角
	float Pitch = 0;
	///偏航角
	float Yaw = -90;
};

glm::mat4 EulerCamera::GetViewModel()
{
	glm::vec3 newFront;
	newFront.x = cos(glm::radians(this->Pitch)) * cos(glm::radians(this->Yaw));
	newFront.y = sin(glm::radians(this->Pitch));
	newFront.z = cos(glm::radians(this->Pitch)) * sin(glm::radians(this->Yaw));
	this->Front = glm::normalize(newFront);
	return glm::lookAt(this->Position, this->Position + this->Front, this->WorldUp);
}

GLuint TexureManager::CreateTexture(char* const pic)
{
	int width, height, nrChannels;
	//opengltexture坐标变换
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(pic, &width, &height, &nrChannels, 0);
	if (data == 0)
	{
		return 0;
	}
	unsigned int TextureBuf;
	glGenTextures(1, &TextureBuf);
	glBindTexture(GL_TEXTURE_2D, TextureBuf);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(data);
	return TextureBuf;
}
bool TexureManager::SwitchTexture(GLuint textureID, GLint layout, int textureunitId)
{
	glActiveTexture(GL_TEXTURE0 + textureunitId);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glUniform1i(layout, textureunitId);
	return true;
}

SampleCamera* cam = NULL;
EulerCamera* eCam = NULL;
int main() 
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_OPENGL_CORE_PROFILE);

	auto windows = glfwCreateWindow(800, 600, "test", NULL, NULL);
	if (windows == NULL) {
		std::cout << "failed to create windows" << std::endl;
		return -1;
	}
	glfwMakeContextCurrent(windows);

	glfwSetFramebufferSizeCallback(windows, framebuffer_size_callback);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	
	/*
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	std::ostringstream* sourcecode = GetShaderSourceFile("c:\\Users\\ka\\Documents\\Visual Studio 2017\\Projects\\ConsoleApp1\\OpenGLTest\\vertex.shader");
	std::string sourcedata = sourcecode->str();
	const char* source_c_str = sourcedata.c_str();
	glShaderSource(vertexShader, 1, &source_c_str, NULL);
	glCompileShader(vertexShader);
	int success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	char errstring[512];
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errstring);
		std::cout << errstring;
	}
	
	VertexShader* vertexShader = new VertexShader("c:\\Users\\ka\\Documents\\Visual Studio 2017\\Projects\\ConsoleApp1\\OpenGLTest\\vertex.shader");
	vertexShader->Init();
	*/
	/*
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	sourcecode = GetShaderSourceFile("c:\\Users\\ka\\Documents\\Visual Studio 2017\\Projects\\ConsoleApp1\\OpenGLTest\\fragment.shader");
	sourcedata = sourcecode->str();
	source_c_str = sourcedata.c_str();
	glShaderSource(fragmentShader, 1, &source_c_str, NULL);
	glCompileShader(fragmentShader);
	success = 0;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errstring);
		std::cout << errstring;
	}
	FragmentShader* fragmentShader = new FragmentShader("c:\\Users\\ka\\Documents\\Visual Studio 2017\\Projects\\ConsoleApp1\\OpenGLTest\\fragment.shader");
	fragmentShader->Init();

	*/
	/*
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader->GetShaderId());
	glAttachShader(shaderProgram, fragmentShader->GetShaderId());
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);
	*/
	ShaderProgramer* programer = new ShaderProgramer(
		"./vertex.shader",
		"./fragment.shader");
	programer->Init();

	ShaderProgramer* lightProgramer = new ShaderProgramer(
		"./lightvertex.shader",
		"./lightfragment.shader");
	lightProgramer->Init();
	programer->UseThisProgram();


	
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f, 
		0.5f,  0.5f, -0.5f, 
		0.5f,  0.5f, -0.5f, 
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f, 
		0.5f,  0.5f,  0.5f, 
		0.5f,  0.5f,  0.5f, 
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		0.5f,  0.5f,  0.5f, 
		0.5f,  0.5f, -0.5f, 
		0.5f, -0.5f, -0.5f, 
		0.5f, -0.5f, -0.5f, 
		0.5f, -0.5f,  0.5f, 
		0.5f,  0.5f,  0.5f, 

		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f, 
		0.5f, -0.5f,  0.5f, 
		0.5f, -0.5f,  0.5f, 
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		0.5f,  0.5f, -0.5f, 
		0.5f,  0.5f,  0.5f, 
		0.5f,  0.5f,  0.5f, 
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
	};
	float vertexMap[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,

		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,

		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,

		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f
	};
	float normals[] = {
		  0.0f,  0.0f, -1.0f,
		 0.0f,  0.0f, -1.0f,
		 0.0f,  0.0f, -1.0f,
		 0.0f,  0.0f, -1.0f,
		  0.0f,  0.0f, -1.0f,
		  0.0f,  0.0f, -1.0f,

		  0.0f,  0.0f, 1.0f,
		 0.0f,  0.0f, 1.0f,
		 0.0f,  0.0f, 1.0f,
		 0.0f,  0.0f, 1.0f,
		  0.0f,  0.0f, 1.0f,
		  0.0f,  0.0f, 1.0f,

		 -1.0f,  0.0f,  0.0f,
		 -1.0f,  0.0f,  0.0f,
		 -1.0f,  0.0f,  0.0f,
		 -1.0f,  0.0f,  0.0f,
		 -1.0f,  0.0f,  0.0f,
		 -1.0f,  0.0f,  0.0f,

		 1.0f,  0.0f,  0.0f,
		 1.0f,  0.0f,  0.0f,
		 1.0f,  0.0f,  0.0f,
		 1.0f,  0.0f,  0.0f,
		 1.0f,  0.0f,  0.0f,
		 1.0f,  0.0f,  0.0f,

		  0.0f, -1.0f,  0.0f,
		 0.0f, -1.0f,  0.0f,
		 0.0f, -1.0f,  0.0f,
		 0.0f, -1.0f,  0.0f,
		  0.0f, -1.0f,  0.0f,
		  0.0f, -1.0f,  0.0f,

		  0.0f,  1.0f,  0.0f,
		 0.0f,  1.0f,  0.0f,
		 0.0f,  1.0f,  0.0f,
		 0.0f,  1.0f,  0.0f,
		  0.0f,  1.0f,  0.0f,
		  0.0f,  1.0f,  0.0f
	};

	unsigned int indices[] = { // 注意索引从0开始! 
		0, 1, 3, // 第一个三角形
		1, 2, 3  // 第二个三角形
	};
	/*
	//VertexBufferObject* vbo1 = new VertexBufferObject("zhengfangxin1", sizeof(vertices), vertices, GL_FLOAT, sizeof(float), 3);
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//VertexBufferObject* vbo2 = new VertexBufferObject("wenli1", sizeof(vertexMap), vertexMap, GL_FLOAT, sizeof(float), 2);
	unsigned int VBO2;
	glGenBuffers(1, &VBO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexMap), vertexMap, GL_STATIC_DRAW);
	//VertexAttributeObject* vao = new VertexAttributeObject();
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//vao->CreateVertexAttribute("aPos", programer, vbo1);
	//vao->CreateVertexAttribute("textPos", programer, vbo2);
	//vao->BindElementBufferObject(sizeof(indices), indices);
	GLint aPos = programer->GetAttLocation("aPos");
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(aPos);

	GLint textPos = programer->GetAttLocation("textPos");
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glVertexAttribPointer(textPos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(textPos);

	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(EBO, sizeof(indices), indices, GL_STATIC_DRAW);
	*/
	/*
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	unsigned int VBO2;
	glGenBuffers(1, &VBO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexMap), vertexMap, GL_STATIC_DRAW);

	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	GLint aPos = programer->GetAttLocation("aPos");
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//0是shader中的layout
	glEnableVertexAttribArray(aPos);

	GLint textPos = programer->GetAttLocation("textPos");
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glVertexAttribPointer(textPos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(textPos);
	*/
	/*
	float vertices[] = {
		//顶点坐标               纹理坐标
		0.5f, 0.5f, 0.0f,//       1.0f, 1.0f,   // 右上角
		0.5f, -0.5f, 0.0f,//      1.0f, 0.0f,  // 右下角
		-0.5f, -0.5f, 0.0f, //    0.0f, 0.0f, // 左下角
		-0.5f, 0.5f, 0.0f, //     0.0f, 1.0f,   // 左上角
	};

	float abc[] = 
	{
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
	};

	unsigned int indices[] = { // 注意索引从0开始! 
		0, 1, 3, // 第一个三角形
		1, 2, 3  // 第二个三角形
	};
	*/
	
	VertexBufferObject* vbo1 = new VertexBufferObject("zhengfangxin1", sizeof(vertices), vertices, GL_FLOAT, sizeof(float), 3);
	VertexBufferObject* vbo2 = new VertexBufferObject("wenli1", sizeof(vertexMap), vertexMap, GL_FLOAT, sizeof(float), 2);
	VertexBufferObject* vbo3 = new VertexBufferObject("normal", sizeof(normals), normals, GL_FLOAT, sizeof(float), 3);
	VertexAttributeObject* vao = new VertexAttributeObject();
	vao->CreateVertexAttribute("aPos", programer, vbo1);
	vao->CreateVertexAttribute("textPos", programer, vbo2);
	vao->CreateVertexAttribute("aNormal", programer, vbo3);
	vao->BindElementBufferObject(sizeof(indices), indices);


	auto textureid = TexureManager::CreateTexture("../resources/container2.png");
	auto spectextureid = TexureManager::CreateTexture("../resources/container2_specular.png");


	VertexAttributeObject* vao1 = new VertexAttributeObject();
	vao1->CreateVertexAttribute("aPos", lightProgramer, vbo1);
	
	


	GLint texturelayout = programer->GetUnifLocation("ourTexture");//glGetUniformLocation(shaderProgram, "ourTexture");
	GLint spetexturelayout = programer->GetUnifLocation("refleTexture");
	//if (texturelayout == -1)
	//	return 0;

	

	auto modelLayout = programer->GetUnifLocation("model");//glGetUniformLocation(shaderProgram, "model");
	if (modelLayout == -1)
		return 0;
	auto lightModelLayout = lightProgramer->GetUnifLocation("model");
	
	auto viewLayout = programer->GetUnifLocation("view");//glGetUniformLocation(shaderProgram, "view");
	if (viewLayout == -1)
		return 0;
	auto lightViewLayout = lightProgramer->GetUnifLocation("view");

	auto projectionLayout = programer->GetUnifLocation("projection");//glGetUniformLocation(shaderProgram, "projection");
	if (projectionLayout == -1)
		return 0;
	auto lightProjectionLayout = lightProgramer->GetUnifLocation("projection");

	auto ambientStrengthLayout = programer->GetUnifLocation("ambientStrength");
	if (ambientStrengthLayout == -1)
		return 0;


	auto lightPositionLayout = programer->GetUnifLocation("lightPosition");
	if (lightPositionLayout == -1)
		return 0;

	auto camPositionLayout = programer->GetUnifLocation("camPos");
	if (camPositionLayout == -1)
		return 0;

	
	cam = new SampleCamera(glm::vec3(0, 0, 3), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
	eCam = new EulerCamera(glm::vec3(0, 0, 3), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));

	glm::vec3 lightPosition(3, 0, -3);
	float ambientStrength = 0.2f;

	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(windows))
	{
		// input
		// -----
		processInput(windows);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glUseProgram(shaderProgram);
		programer->UseThisProgram();
		TexureManager::SwitchTexture(textureid, texturelayout, 0);
		TexureManager::SwitchTexture(spectextureid, spetexturelayout, 1);
		//glBindVertexArray(VAO);
		vao->UseThisVAO();

		glm::mat4 model;
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLayout, 1, GL_FALSE, glm::value_ptr(model));

		glm::mat4 view;
		//view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
		view = eCam->GetViewModel();//cam->GetViewModel();
		glUniformMatrix4fv(viewLayout, 1, GL_FALSE, glm::value_ptr(view));

		auto campos = eCam->GetCamPosition();
		glUniform3f(camPositionLayout, campos.x, campos.y, campos.z);

		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)800 / (float)600, 0.1f, 100.0f);
		glUniformMatrix4fv(projectionLayout, 1, GL_FALSE, glm::value_ptr(projection));

		glUniform1f(ambientStrengthLayout, ambientStrength);
		glUniform3f(lightPositionLayout, lightPosition.x, lightPosition.y, lightPosition.z);



		glDrawArrays(GL_TRIANGLES, 0, 36);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		vao1->UseThisVAO();
		lightProgramer->UseThisProgram();
		glm::mat4 newmodel;
		newmodel = glm::translate(newmodel, lightPosition);
		newmodel = glm::scale(newmodel, glm::vec3(0.2, 0.2, 0.2));
		glUniformMatrix4fv(lightModelLayout, 1, GL_FALSE, glm::value_ptr(newmodel));
		glUniformMatrix4fv(lightViewLayout, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(lightProjectionLayout, 1, GL_FALSE, glm::value_ptr(projection));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(windows);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();

	return 0;
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		eCam->PitchUp();// cam->MoveFront();
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		eCam->PitchDown();//cam->MoveBack();
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		eCam->YawLeft();
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		eCam->YawRight();

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		eCam->MoveFront();
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		eCam->MoveBack();
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		eCam->MoveRight();
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		eCam->MoveLeft();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
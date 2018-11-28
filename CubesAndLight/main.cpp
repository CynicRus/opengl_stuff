#define GLEW_STATIC
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"


#include <iostream>
#include <vector>



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);          
#endif

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Cubes and Light", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glewInit();

	glEnable(GL_DEPTH_TEST);

	Shader lightingShader("shaders/default.vert", "shaders/default.frag");
	Shader lampShader("shaders/lamp.vert", "shaders/lamp.frag");

	float vertices[] = {
	-0.5, -0.5, -0.5, 0.5,  0.5,  -0.5, 0.5,  -0.5, -0.5, -0.5, -0.5, -0.5,
	-0.5, 0.5,  -0.5, 0.5,  0.5,  -0.5, -0.5, -0.5, -0.5, -0.5, 0.5,  0.5,
	-0.5, 0.5,  -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5,  -0.5, 0.5,  0.5,
	-0.5, 0.5,  -0.5, 0.5,  0.5,  0.5,  0.5,  0.5,  -0.5, -0.5, 0.5,  -0.5,
	-0.5, 0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  -0.5, -0.5, 0.5,  0.5,  -0.5,
	0.5,  0.5,  0.5,  0.5,  -0.5, -0.5, 0.5,  0.5,  0.5,  0.5,  -0.5, 0.5,
	-0.5, -0.5, -0.5, 0.5,  -0.5, -0.5, 0.5,  -0.5, 0.5,  -0.5, -0.5, -0.5,
	0.5,  -0.5, 0.5,  -0.5, -0.5, 0.5,  -0.5, -0.5, 0.5,  0.5,  -0.5, 0.5,
	0.5,  0.5,  0.5,  -0.5, -0.5, 0.5,  0.5,  0.5,  0.5,  -0.5, 0.5,  0.5 };

	float normals[] = {
		0,  0,  -1, 0,  0,  -1, 0,  0,  -1, 0,  0,  -1, 0,  0,  -1, 0,  0,  -1,
		-1, 0,  0,  -1, 0,  0,  -1, 0,  0,  -1, 0,  0,  -1, 0,  0,  -1, 0,  0,
		0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,
		1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,
		0,  -1, 0,  0,  -1, 0,  0,  -1, 0,  0,  -1, 0,  0,  -1, 0,  0,  -1, 0,
		0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1 };
	
	unsigned int VBO, VAO, VNormals;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &VNormals);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, VNormals);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	lightingShader.use();
	lightingShader.setVec3("objectColor", 2.0f, 0.5f, 0.31f);
	lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);

	const double d = 1.8;     

	const int s = 4;
	const int w = 12;
	
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);       
		glEnable(GL_DEPTH_TEST);  
		glDepthFunc(GL_LESS);

		lightingShader.use();
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glm::vec3 lightInvDir = glm::vec3(1.0, 0, 0);
		GLfloat radius = 40.0f;
		GLfloat camX = 20;
		GLfloat camY = 20;
		GLfloat camZ = -3.0;

		glm::vec3 lightPos(sin(glfwGetTime()) * radius, cos(glfwGetTime()) * radius, -3.0f);
		lightingShader.setVec3("lightPos", lightPos);

		lightingShader.setVec3("viewPos", glm::vec3(camX,camY,camZ));
		glm::mat4 view;
	    view = glm::lookAt(glm::vec3(camX, camY, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 ortho = glm::ortho<float>(-w, w, -w, +w, -w, +w);
	    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		

		unsigned int modelLoc = glGetUniformLocation(lightingShader.ID, "model");
		unsigned int viewLoc = glGetUniformLocation(lightingShader.ID, "view");
		unsigned int orthoLoc = glGetUniformLocation(lightingShader.ID, "projection");
		glBindVertexArray(VAO);

		for (int stop = s, x = -s; x <= stop; x++)
		{
			for (int stop = s, y = -s; y <= stop; y++)
			{
				for (int stop = s, z = -s; z <= stop; z++)
				{
					glm::mat4 model = glm::mat4(1.0);
					model = glm::translate(model,glm::vec3(x * d, y * d, z * d)); 
					model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));  
					
					glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
					glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
					glUniformMatrix4fv(orthoLoc, 1, GL_FALSE, glm::value_ptr(projection));
					glDrawArrays(GL_TRIANGLES, 0, 36);

				}
			}
		}

		lampShader.use();
		lampShader.setMat4("projection", projection);
		lampShader.setMat4("view", view);
		glm::mat4 model = glm::mat4();
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));    
		lampShader.setMat4("model", model);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);



		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1,&VNormals);

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
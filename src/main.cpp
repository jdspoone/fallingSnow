#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS //In GLM taking degrees as params is deprecated
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

GLFWwindow* window = NULL;
GLuint program;
GLuint vao, vbo, tbo;

/*
*	Loads GLSL shaders
*	Credit to NeHe OpenGL Tutorials for this code
*/
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path)
{

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
	std::string Line = "";
	while(getline(VertexShaderStream, Line))
	    VertexShaderCode += "\n" + Line;
	VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
	std::string Line = "";
	while(getline(FragmentShaderStream, Line))
	    FragmentShaderCode += "\n" + Line;
	FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "Vert Shader Check: %s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "Frag Shader Check: %s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "Linking Check: %s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}


// Keyboard callback function.
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

/*
* Draw Calls
*/
void Render()
{
  glClearColor(0.6,0.6,0.6,0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(program);
  
  glfwSwapBuffers(window);

 
  glUseProgram(0);

}

/*
* Model-View-Projection Transformation Matrix
*/
void updateMVP()
{
  //Camera
  glm::vec3 cameraPosition = glm::vec3(4.0f,3.0f,3.0f);
  glm::vec3 cameraTarget = glm::vec3(0.0f,0.0f,0.0f);
  glm:: vec3 upVector = glm::vec3(0.0f,1.0f,0.0f);
  glm::mat4 CameraMatrix = glm::lookAt(cameraPosition, cameraTarget, upVector);

  //Projection
  float fovy = M_PI * 0.25f; //Radians,this is equivalent to 45 degrees
  float aspect = 1.0f;
  float zNear = 0.1f;
  float zFar = 100.0f;
  glm::mat4 ProjectionMatrix = glm::perspective(fovy, aspect, zNear, zFar);

  //Model
  //nothing to do here yet
  glm::mat4 ModelMatrix = glm::mat4(1.0f);  //Identity Matrix

  //MVP
  glm::mat4 MVP = ProjectionMatrix * CameraMatrix * ModelMatrix;

  //Pass through to shaders
  glUseProgram(program); //bind
  GLuint MatrixID = glGetUniformLocation(program, "MVP"); 
  glUniformMatrix4fv(MatrixID,  1, GL_FALSE, &MVP[0][0]);
  glUseProgram(0); //unbind
   
}

int main(int argc, char *argv[])
{
  // Initialize GLFW
  if (!glfwInit()) {
    printf("glfwInit failed\n");
    exit(EXIT_FAILURE);
  }
  
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  // Create our window
  window = glfwCreateWindow(800, 800, "Falling Snow", NULL, NULL);
  if (!window) {
    glfwTerminate();
    printf("glfw window create failed\n");
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);

  // Initialize GLEW
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if(err != GLEW_OK){
    printf("glewInit failed\n");
    exit(EXIT_FAILURE);
  }
  
  //Loading shaders, will need to modify and add Geometry Shader
  program = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");

  const GLchar* feedbackVaryings[] = { "outVec" };
  glTransformFeedbackVaryings(program, 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
  
  glLinkProgram(program);
  glUseProgram(program);

  // Create VAO
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Create input VBO and vertex format
  vector<glm::vec3> points;
  points.push_back(glm::vec3(0.0f, 0.0f, 0.0f)); //point at origin

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * points.size() * sizeof(glm::vec3), NULL, GL_DYNAMIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * points.size() * sizeof(glm::vec3), &points[0][0]);

  GLint inputAttrib = glGetAttribLocation(program, "inVec");
  //glEnableVertexAttribArray(inputAttrib);
  //glVertexAttribPointer(inputAttrib, 4, GL_FLOAT, GL_FALSE, 0, 0);

  // Create transform feedback buffer
  glGenBuffers(1, &tbo);
  glBindBuffer(GL_ARRAY_BUFFER, tbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * points.size() * sizeof(glm::vec4), NULL, GL_DYNAMIC_READ);

  glEnable(GL_PROGRAM_POINT_SIZE); //Will remove after geometry shader is implemented
  if(program) {
    for (int i = 0; i < 5; ++i) {
    
      // Re-bind our input buffer
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glEnableVertexAttribArray(inputAttrib);
      glVertexAttribPointer(inputAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

      // Re-bind our output buffer
      glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);

      // Perform the feedback transform
      glBeginTransformFeedback(GL_POINTS);
      glDrawArrays(GL_POINTS, 0, (int)points.size());
      glEndTransformFeedback();
      glFlush();

      // Fetch and print results
      GLfloat buf[3];
      glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(buf), buf);
      printf("%f %f %f\n", buf[0], buf[1], buf[2]);
			
      // Swap the 2 buffers
      std::swap(vbo, tbo);
    }
  }


  updateMVP(); //No movement yet, just static camera	
  while(!glfwWindowShouldClose(window))
  {
    glfwPollEvents();	//For key & mouse events
	Render();	//Do all the things

  }

  //Cleanup 
  glDeleteProgram(program);
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}


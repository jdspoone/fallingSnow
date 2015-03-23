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
#include <algorithm>

#ifdef _WIN32
#define M_PI 3.14159265358979323846f
#endif

using namespace std;

GLFWwindow* window = NULL;
vector<glm::vec4> points;
GLuint snowProgram, feedbackProgram;
GLuint vertexLocation;
GLuint vao, vbo, tbo;

GLuint loadShader(GLenum type, const GLchar *path)
{
  // For logging purposes...
  const char *typeName;
  switch (type) {
  case 0x8B31:
    typeName = "vertex";
    break;
  case 0x8DD9:
    typeName = "geometry";
    break;
  case 0x8B30:
    typeName = "fragment";
    break;
  default:
    typeName = "unknown";
    break;
  }

  // Create the shader
  GLuint shader = glCreateShader(type);
  
  // Load the shader code from the given file path
	std::string shaderCode;
	std::ifstream shaderStream(path, std::ios::in);
	if(shaderStream.is_open()) {
    std::string Line = "";
    while(getline(shaderStream, Line))
        shaderCode += "\n" + Line;
    shaderStream.close();
	}
	else {
		printf("Could not open %s shader file: %s\n", typeName, path);
	}
  
	GLint result = GL_FALSE;
	int logLength;
  
	// Compile the shader
	printf("Compiling %s shader: %s\n", typeName, path);
	char const * shaderSourcePointer = shaderCode.c_str();
	glShaderSource(shader, 1, &shaderSourcePointer , NULL);
	glCompileShader(shader);

	// Check Vertex Shader
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
	std::vector<char> shaderErrorMessage(logLength);
	glGetShaderInfoLog(shader, logLength, NULL, &shaderErrorMessage[0]);
	fprintf(stdout, "Checking %s shader: %s\n", typeName, &shaderErrorMessage[0]);
  
  return shader;
}


GLuint loadSnowShaders(const char *vPath, const char *gPath, const char *fPath)
{
	// Create the shaders
	GLuint vertexID = loadShader(GL_VERTEX_SHADER, vPath);
	GLuint geometryID = loadShader(GL_GEOMETRY_SHADER, gPath);
	GLuint fragmentID = loadShader(GL_FRAGMENT_SHADER, fPath);

	// Link the program
	fprintf(stdout, "Linking snow program\n");
	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertexID);
	glAttachShader(programID, geometryID);
	glAttachShader(programID, fragmentID);
	glLinkProgram(programID);

	GLint result = GL_FALSE;
	int logLength;

	// Check the program
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLength);
	std::vector<char> errorMessage( max(logLength, int(1)) );
	glGetProgramInfoLog(programID, logLength, NULL, &errorMessage[0]);
	fprintf(stdout, "Linking Check: %s\n", &errorMessage[0]);

	glDeleteShader(vertexID);
	glDeleteShader(geometryID);
	glDeleteShader(fragmentID);

	return programID;
}

// Keyboard callback function.
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}


GLuint loadFeedbackShader(const char *vPath)
{
	// Create the shader
	GLuint vertexID = loadShader(GL_VERTEX_SHADER, vPath);

	// Link the program
	fprintf(stdout, "Linking feedback program\n");
	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertexID);
	glLinkProgram(programID);

	GLint result = GL_FALSE;
	int logLength;

	// Check the program
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLength);
	std::vector<char> errorMessage( max(logLength, int(1)) );
	glGetProgramInfoLog(programID, logLength, NULL, &errorMessage[0]);
	fprintf(stdout, "Linking Check: %s\n", &errorMessage[0]);

	glDeleteShader(vertexID);

	return programID;
}


/*
* Draw Calls
*/
void Render()
{
  glClearColor(0.6f,0.6f,0.6f,0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(snowProgram);

  glBindVertexArray(vao);
  glEnableVertexAttribArray(vertexLocation);
  glVertexAttribPointer(vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glDrawArrays(GL_POINTS, 0, (int)points.size());

  glBindVertexArray(0);
  glUseProgram(0);
  
  glfwSwapBuffers(window);
}

void LoadPoints()
{
  //Create Points
  float x,y,z,velocity = 1.0f;
  /* initialize random seed: */
  srand (time(NULL));
 for (float k = -1; k <= 1; k+= 0.1f)
  for (float i = -1; i <= 1; i+= 0.1f)
    for (float j = -1; j <= 1; j+= 0.1f)
    {
        x = ((rand() % 10 + 9)/100.0f) + k; 
        y = ((rand() % 10 + 9)/100.0f) + i; 
        z = ((rand() % 10 + 6)/100.0f) + j;
        velocity = ((rand() % 10 + 9)/1000.0f); 
        points.push_back(glm::vec4(x,y,z,velocity));
    }

   cout <<"Particle Count: "<<points.size()<<endl;
  //Attach to buffer and vao
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(glm::vec4), &points[0][0]);

  glBindVertexArray(0); 
}

void Feedback()
{
  glEnable(GL_RASTERIZER_DISCARD);
  glUseProgram(feedbackProgram); //Bind
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, tbo);

  // Re-bind our output buffer
  glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);
  glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec4), NULL, GL_DYNAMIC_READ);
      
  // Perform the feedback transform
  glBeginTransformFeedback(GL_POINTS);
  glDrawArrays(GL_POINTS, 0, (int)points.size());
  glEndTransformFeedback();
  glFlush();
			
  // Swap the 2 buffers
  std::swap(vbo, tbo);
	  
  glDisable(GL_RASTERIZER_DISCARD);
  
  glBindVertexArray(0);
  glUseProgram(0); //Unbind
}

/*
* Model-View-Projection Transformation Matrix
*/
void LoadMVP()
{
  //Camera
  glm::vec3 cameraPosition = glm::vec3(0.0f,3.0f,3.0f);
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
  glUseProgram(snowProgram); //bind
  GLuint MatrixID = glGetUniformLocation(snowProgram, "MVP");
  glUniformMatrix4fv(MatrixID,  1, GL_FALSE, &MVP[0][0]);
  glUseProgram(0); //unbind
   
}


void setupRenderingContext()
{
  snowProgram = loadSnowShaders("vertexShader.glsl", "geometryShader.glsl", "fragmentShader.glsl");

  feedbackProgram = loadFeedbackShader("feedbackShader.glsl");
  const GLchar* feedbackVaryings[] = { "outVec" };
  glTransformFeedbackVaryings(feedbackProgram, 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
  glLinkProgram(feedbackProgram);

  //Where to pass in vertices to the shaders
  vertexLocation = glGetAttribLocation(snowProgram, "inVec");

  // Create VAO
  glGenVertexArrays(1, &vao);

  // Create VBO
  glGenBuffers(1, &vbo); //Attatched to VAO in LoadPoints(), and Render()

  // Create TBO
  glGenBuffers(1, &tbo); //Attatched to VAO in Feedback()

  glEnable(GL_PROGRAM_POINT_SIZE); //Will remove after geometry shader is implemented, maybe
  glEnable(GL_DEPTH_TEST);
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
  
  /*
  * Sets up VAO, VBO, and TBO
  */
  setupRenderingContext(); 
  
  /*
  * Sets up Model-View-Projection transformation matrix
  * TODO: Controllable camera
  */
  LoadMVP();

  /*
  * Loads the Snow Particles
  * Fixed amount
  */
  LoadPoints(); 

  while(!glfwWindowShouldClose(window))
  {
    /*
    * Polls for Mouse & Keyboard Events
    */
    glfwPollEvents();

    /*
    * Draws scene to screen
    */
    Render();

    /*
    * GPU acceleration
    * Gets back vectors after being transformed by shaders.
    * TODO: Bounds checking, Particle insertion & deletion, sorting
    *       (consider OpenCL/CUDA for CPU accelerated inspection)
    */
    Feedback(); 

  }

  //Cleanup 
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &tbo);
  glDeleteVertexArrays(1, &vao);
  glDeleteProgram(snowProgram);
  glDeleteProgram(feedbackProgram);
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}


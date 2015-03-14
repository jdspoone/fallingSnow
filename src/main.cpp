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

GLuint program;
GLuint vao, vbo, tbo;

// Load the shader from the specified file. Returns false if the shader could not be loaded
static GLubyte shaderText[8192];
bool loadShaderFile(const char *filename, GLuint shader) {
  GLint shaderLength = 0;
  FILE *fp;
  
  // Open the shader file
  fp = fopen(filename, "r");
  if(fp != NULL) {
    // See how long the file is
    while (fgetc(fp) != EOF)
      shaderLength++;
    
    // Go back to beginning of file
    rewind(fp);
    
    // Read the whole file in
    fread(shaderText, 1, shaderLength, fp);
    
    // Make sure it is null terminated and close the file
    shaderText[shaderLength] = '\0';
    fclose(fp);
  }
  else {
    return false;
  }
  
  // Load the string into the shader object
  GLchar* fsStringPtr[1];
  fsStringPtr[0] = (GLchar *)((const char*)shaderText);
  glShaderSource(shader, 1, (const GLchar **)fsStringPtr, NULL);
  
  return true;
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
void Render(GLFWwindow* window)
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
  glm::mat4 CameraMatrix = glm::lookAt( cameraPosition, cameraTarget, upVector);

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
  GLFWwindow* window;
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
  
  //================== Vertex Shader ===========================
  // Compile vertex shader
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  if(!loadShaderFile("vertexShader.glsl", vertex_shader)) {
    glDeleteShader(vertex_shader);
    cout << "The vertex shader could not be found" << endl;
  }
  glCompileShader(vertex_shader);
  
  //================== Fragment Shader ========================
  // Compile vertex shader
  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  if(!loadShaderFile("vertexShader.glsl", fragment_shader)) {
    glDeleteShader(fragment_shader);
    cout << "The fragment shader could not be found" << endl;
  }
  glCompileShader(fragment_shader);

 
  //================== Geometry Shader ========================
  //TODO

  //================= Link Shaders ============================ 
  // Create program and specify transform feedback variables
  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  
  const GLchar* feedbackVaryings[] = { "outValue" };
  glTransformFeedbackVaryings(program, 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
  
  glLinkProgram(program);
  glUseProgram(program);

  // Create VAO
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Create input VBO and vertex format
  vector<glm::vec4> points;
  points.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); //point at origin

  /*
  vector<GLfloat> points = vector<GLfloat>(4);
  points[0] = 1.0f;
  points[1] = 2.0f;
  points[2] = 3.0f;
  points[3] = 4.0f;
  */

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * points.size() * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * points.size() * sizeof(glm::vec4), &points[0][0]);

  GLint inputAttrib = glGetAttribLocation(program, "inValue");
  glEnableVertexAttribArray(inputAttrib);
  glVertexAttribPointer(inputAttrib, 4, GL_FLOAT, GL_FALSE, 0, 0);

  // Create transform feedback buffer
  glGenBuffers(1, &tbo);
  glBindBuffer(GL_ARRAY_BUFFER, tbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * points.size() * sizeof(glm::vec4), NULL, GL_DYNAMIC_READ);

  // We aren't interested in drawing anything at the moment...
  //glEnable(GL_RASTERIZER_DISCARD);


  //Build the scene here, for now
  glEnable(GL_PROGRAM_POINT_SIZE); //Will remove after geometry shader is implemented
  if(program) {
    for (int i = 0; i < 5; ++i) {
    
      // Re-bind our input buffer
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glEnableVertexAttribArray(inputAttrib);
      glVertexAttribPointer(inputAttrib, 4, GL_FLOAT, GL_FALSE, 0, 0);

      // Re-bind our output buffer
      glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);

      // Perform the feedback transform
      glBeginTransformFeedback(GL_POINTS);
      glDrawArrays(GL_POINTS, 0, (int)points.size());
      glEndTransformFeedback();
      glFlush();

      // Fetch and print results
      GLfloat buf[4];
      glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(buf), buf);
      printf("%f %f %f %f\n", buf[0], buf[1], buf[2], buf[3]);
			
      // Swap the 2 buffers
      std::swap(vbo, tbo);
    }
  }

  updateMVP(); //No movement yet, just static camera	
  while(!glfwWindowShouldClose(window))
  {
    glfwPollEvents();	//For key & mouse events
	Render(window);	//Do all the things

  }

  //Cleanup 
  glDeleteProgram(program);
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}


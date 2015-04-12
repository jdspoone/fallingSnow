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
#include <time.h>
#include "lodepng.h" //Credit: http://lodev.org/lodepng/

#ifdef _WIN32
#define M_PI 3.14159265358979323846f
#endif

using namespace std;

GLFWwindow* window = NULL;
GLuint Frames = 0;
double Timer = glfwGetTime();
vector<glm::vec3> positions;
vector<glm::vec3> velocities;
vector<GLfloat> rotationAngles;
GLuint snowProgram, feedbackProgram, backdropProgram, floorProgram;
GLuint renderPosition, renderVelocity, renderRotation;
GLuint feedbackPosition, feedbackVelocity, feedbackRotation;
GLuint snowVAO;
GLuint positionVBO[2];
GLuint velocityVBO[2];
GLuint backTID, floorTID;

unsigned int iteration = 0;

//====== Camera Settings =======
glm::vec3 cameraPosition = glm::vec3(0.0f); //initial starting position
glm::vec3 cameraDirection;
glm::vec3 cameraRight; 
GLfloat camera_step = 0.01f;
GLfloat cameraTheta, cameraPhi = 0.0f;

bool ScreenLock = false;
int ScreenWidth, ScreenHeight;

unsigned int particleCount;
unsigned int particleStep = 5000;
int maxChangePerFrame = 1000;

//Scene Toggles
bool key_one = true;
bool key_two = false;

#pragma mark - Shaders

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
	if ( logLength > 0)
        fprintf(stdout, "Checking %s shader: %s\n", typeName, &shaderErrorMessage[0]);
  
  return shader;
}


GLuint loadFeedbackShader(const char *vPath)
{
	// Create the shader
	GLuint vertexID = loadShader(GL_VERTEX_SHADER, vPath);

	// Link the program
	fprintf(stdout, "Linking feedback program\n\n");
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
	if ( logLength > 0)
	    fprintf(stdout, "Linking Check: %s\n", &errorMessage[0]);

	glDeleteShader(vertexID);

	return programID;
}


GLuint loadShadersVGF(const char *vPath, const char *gPath, const char *fPath)
{
	// Create the shaders
	GLuint vertexID = loadShader(GL_VERTEX_SHADER, vPath);
	GLuint geometryID = loadShader(GL_GEOMETRY_SHADER, gPath);
	GLuint fragmentID = loadShader(GL_FRAGMENT_SHADER, fPath);

	// Link the program
	fprintf(stdout, "Linking Vertex-Geomentry-Fragment program\n\n");
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
	if ( logLength > 0 )
	    fprintf(stdout, "Linking Check: %s\n", &errorMessage[0]);

	glDeleteShader(vertexID);
	glDeleteShader(geometryID);
	glDeleteShader(fragmentID);

	return programID;
}


GLuint loadShadersVF(const char *vPath, const char *fPath)
{
	// Create the shaders
	GLuint vertexID = loadShader(GL_VERTEX_SHADER, vPath);
	GLuint fragmentID = loadShader(GL_FRAGMENT_SHADER, fPath);

	// Link the program
	fprintf(stdout, "Linking Vertex-Fragment program\n\n");
	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertexID);
	glAttachShader(programID, fragmentID);
	glLinkProgram(programID);

	GLint result = GL_FALSE;
	int logLength;

	// Check the program
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLength);
	std::vector<char> errorMessage( max(logLength, int(1)) );
	glGetProgramInfoLog(programID, logLength, NULL, &errorMessage[0]);
	if ( logLength > 0 )
	    fprintf(stdout, "Linking Check: %s\n", &errorMessage[0]);

	glDeleteShader(vertexID);
	glDeleteShader(fragmentID);

	return programID;
}


#pragma mark - OpenGL

void setupRenderingContext()
{
  //Load vertex, geomtry, and fragment shaders for snow
  snowProgram = loadShadersVGF("Shaders/Snow/vertex.glsl", "Shaders/Snow/geometry.glsl", "Shaders/Snow/fragment.glsl");

  //for the floor
  floorProgram = loadShadersVF("Shaders/Floor/vertex.glsl", "Shaders/Floor/fragment.glsl");

  //Load vertex, and fragment shaders for plane textures
  backdropProgram = loadShadersVF("Shaders/Backdrop/vertex.glsl", "Shaders/Backdrop/fragment.glsl");
  glGenTextures(1, &backTID); //load back texture ID

  //Load vertex shader to preform feedback transformations on
  feedbackProgram = loadFeedbackShader("Shaders/Feedback/vertex.glsl");
  const GLchar* feedbackVaryings[] = { "nextPosition", "nextVelocity" };
  glTransformFeedbackVaryings(feedbackProgram, 2, feedbackVaryings, GL_SEPARATE_ATTRIBS);
  glLinkProgram(feedbackProgram);

  // Attribute bindings
  renderPosition = glGetAttribLocation(snowProgram, "position");
  renderVelocity = glGetAttribLocation(snowProgram, "velocity");
  renderRotation = glGetAttribLocation(snowProgram, "rotation");

  feedbackPosition = glGetAttribLocation(feedbackProgram, "previousPosition");
  feedbackVelocity = glGetAttribLocation(feedbackProgram, "previousVelocity");
  feedbackRotation = glGetAttribLocation(feedbackProgram, "previousRotation");

  // Generate our vertex array object
  glGenVertexArrays(1, &snowVAO);

  // Generate our vertex buffer objects
  glGenBuffers(2, positionVBO);
  glGenBuffers(2, velocityVBO);
  
  glEnable(GL_DEPTH_TEST);
}


void LoadPoints()
{
  //Create Points
  float x,y,z = 1.0f;
  /* initialize random seed: */
  srand ((unsigned int)time(NULL));
  for (float k = -1; k <= 1; k+= 0.03f)
    for (float i = -1; i <= 1; i+= 0.03f)
      for (float j = -1; j <= 1; j+= 0.03f) {
        // Generate numbers in the range of 0.09 to 0.18, then offset by k/i for
        // final numbers in the range -0.91 to 1.18
        x = ((rand() % 10 + 9)/100.0f) + k; 
        y = ((rand() % 10 + 9)/100.0f) + i; 
        // Range is -0.94 to 1.15
        z = ((rand() % 10 + 6)/100.0f) + j;
        positions.push_back(glm::vec3(x,y,z));
      }
  
  particleCount = (unsigned int)positions.size();
  cout <<"Particle Count: " << particleCount << endl;
  
  // Create our initial per-particle velocities
  for (unsigned int i = 0; i < particleCount; ++i) {
    velocities.push_back(glm::vec3(0.0, -0.0001, 0.0));
  }

  // Allocate and initialize the position vertex buffer
  for (int i = 0; i < 2; ++i) {
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO[i]);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), &positions[0][0], GL_DYNAMIC_DRAW);
  }
  
  // Allocate and initialize the position vertex buffer
  for (int i = 0; i < 2; ++i) {
    glBindBuffer(GL_ARRAY_BUFFER, velocityVBO[i]);
    glBufferData(GL_ARRAY_BUFFER, velocities.size() * sizeof(glm::vec3), &velocities[0][0], GL_DYNAMIC_DRAW);
  }
}


void Render()
{
  glClearColor(0.6f,0.6f,0.6f,0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Bind the shader program
  glUseProgram(snowProgram);

  // Bind the VAO
  glBindVertexArray(snowVAO);
  
  // Establish the necessary attribute bindings for rendering
  glBindBuffer(GL_ARRAY_BUFFER, positionVBO[iteration % 2]);
  glEnableVertexAttribArray(renderPosition);
  glVertexAttribPointer(renderPosition, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
  
  glBindBuffer(GL_ARRAY_BUFFER, velocityVBO[iteration % 2]);
  glEnableVertexAttribArray(renderVelocity);
  glVertexAttribPointer(renderVelocity, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
  
  // Render snowflakes to screen
  glDrawArrays(GL_POINTS, 0, (int)positions.size());

  // Disable the attributes used for rendering
  glDisableVertexAttribArray(renderPosition);
  glDisableVertexAttribArray(renderVelocity);

  // Unbind the VAO
  glBindVertexArray(0);
  
  // Unbind the shader program
  glUseProgram(0);
  
  glfwSwapBuffers(window);
}


void Feedback()
{
  glEnable(GL_RASTERIZER_DISCARD);
  
  // Bind the transform feedback program
  glUseProgram(feedbackProgram);
  
  // Bind the VAO
  glBindVertexArray(snowVAO);

  // Establish the necessary attribute bindings for transform feedback
  glBindBuffer(GL_ARRAY_BUFFER, positionVBO[iteration % 2]);
  glEnableVertexAttribArray(feedbackPosition);
  glVertexAttribPointer(feedbackPosition, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
  
  glBindBuffer(GL_ARRAY_BUFFER, velocityVBO[iteration % 2]);
  glEnableVertexAttribArray(feedbackVelocity);
  glVertexAttribPointer(feedbackVelocity, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);

  // Re-bind our output VBO
  glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, positionVBO[(iteration + 1) % 2]);
  glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velocityVBO[(iteration + 1) % 2]);

  // Perform the feedback transform
  glBeginTransformFeedback(GL_POINTS);
  glDrawArrays(GL_POINTS, 0, (int)positions.size());
  glEndTransformFeedback();
  glFlush();
			
  // Swap the 2 buffers
  std::swap(positionVBO[0], positionVBO[1]);
  std::swap(velocityVBO[0], velocityVBO[1]);
  
  // Disable the attributes used in transform feedback
  glDisableVertexAttribArray(feedbackPosition);
  glDisableVertexAttribArray(feedbackVelocity);

  // Unbind the VAO
  glBindVertexArray(0);
  
  // Unbind the transform feedback program
  glUseProgram(0);
  
  glDisable(GL_RASTERIZER_DISCARD);
}


void LoadMVP()
{
  //Camera
  cameraDirection = glm::vec3(cos(cameraTheta) * sin(cameraPhi),
                                       sin(cameraTheta),
                                       cos(cameraTheta) * cos(cameraPhi));
  glm::vec3 cameraTarget = cameraPosition + cameraDirection; 
  //cout<<"Camera Target: "<<cameraTarget.x<<","<<cameraTarget.y<<","<<cameraTarget.z<<endl; 
 
  cameraRight = glm::vec3( sin(cameraPhi - M_PI/2.0f),
                           0,
                           cos(cameraPhi - M_PI/2.0f));

  glm:: vec3 upVector = glm::cross( cameraRight, cameraDirection );
  glm::mat4 CameraMatrix = glm::lookAt(cameraPosition, cameraTarget, upVector);

  //Projection
  float fovy = M_PI * 0.25f; //Radians,this is equivalent to 45 degrees
  float aspect = 1.0f;
  float zNear = 0.0001f;
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
  
  glUseProgram(backdropProgram); //bind
  MatrixID = glGetUniformLocation(backdropProgram, "MVP");
  glUniformMatrix4fv(MatrixID,  1, GL_FALSE, &MVP[0][0]);
  
  glUseProgram(floorProgram); //bind
  MatrixID = glGetUniformLocation(floorProgram, "MVP");
  glUniformMatrix4fv(MatrixID,  1, GL_FALSE, &MVP[0][0]);
  
  glUseProgram(0); //unbind
   
}


// Loads a PNG from file, and adds info to texture
void LoadTexture(const char* filename, GLuint textureID, GLuint shaderID)
{
	/*
	* Load Image
	*/
	vector<unsigned char> image; //the raw pixels
  	unsigned width, height;

  	//decode
  	unsigned error = lodepng::decode(image, width, height, filename);
	cout<< filename <<" >> height: "<<height<<", width: "<<width<<endl;

  	//if there's an error, display it
  	if(error) cout << "decoder error " << error << ": " 
		<< lodepng_error_text(error) << endl;
	
	//Bind
	glBindTexture(GL_TEXTURE_2D, textureID);

	//Load image into texture
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

	//For sampling
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

}


#pragma mark - GLFW


// Event handler for mouse clicks
void MouseButton(GLFWwindow * window, int button, int action, int mods)
{
	if (action == GLFW_PRESS) {
		ScreenLock = true;
  }
	if (action == GLFW_RELEASE) {
		ScreenLock = false;
    glfwSetCursorPos(window, ScreenWidth/2.0, ScreenHeight/2.0);
  }
}


// Handler for keeping track of mouse position
void CursorPos(GLFWwindow * window, double xpos, double ypos)
{
	//Check if holding down mouse button
	if (ScreenLock) {
	   cameraPhi   +=  0.00005 * (ScreenWidth/2.0 - xpos);
	   cameraTheta +=  0.00005 * (ScreenHeight/2.0 - ypos);
	}
}


// Keyboard callback function.
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
  if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
    cameraPosition += cameraDirection * camera_step;
  if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
    cameraPosition -= cameraDirection * camera_step;
  if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
    cameraPosition -= cameraRight * camera_step;
  if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
    cameraPosition += cameraRight * camera_step;
  if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
    particleCount += particleStep;
  if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
    particleCount = std::max(particleCount - particleStep, 0u);
  if (key == GLFW_KEY_1 && action == GLFW_PRESS)
      key_one = !key_one;
  if (key == GLFW_KEY_2 && action == GLFW_PRESS)
      key_two = !key_two;
}


void FPS()
{
	double elapsed = glfwGetTime() - Timer;
	if (elapsed > 1.0) {
		char title[32];
		sprintf(title,"Falling Snow, FPS: %0.2f",Frames/elapsed);
		glfwSetWindowTitle(window,title);
		Timer = glfwGetTime();
		Frames = 0;
	}
    else {
    Frames++;
  }
}


#pragma mark - main


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
  ScreenWidth = 800;
  ScreenHeight = 800;
  window = glfwCreateWindow(ScreenWidth, ScreenHeight, "Falling Snow", NULL, NULL);
  if (!window) {
    glfwTerminate();
    printf("glfw window creation failed\n");
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);
  glfwSetMouseButtonCallback(window, MouseButton);
  glfwSetCursorPosCallback(window, CursorPos);

  // Initialize GLEW
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if(err != GLEW_OK){
    printf("glewInit failed\n");
    exit(EXIT_FAILURE);
  }
  
  setupRenderingContext();
  LoadMVP();
  LoadPoints();
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  while(!glfwWindowShouldClose(window)) {
    FPS();
    glfwPollEvents();
    LoadMVP();
    Render();
    Feedback();

  }

  //Cleanup 
  glDeleteBuffers(2, positionVBO);
  glDeleteBuffers(2, velocityVBO);
  glDeleteVertexArrays(1, &snowVAO);
  glDeleteProgram(snowProgram);
  glDeleteProgram(backdropProgram);
  glDeleteProgram(floorProgram);
  glDeleteProgram(feedbackProgram);
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}


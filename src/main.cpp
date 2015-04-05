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

#ifdef _WIN32
#define M_PI 3.14159265358979323846f
#endif

using namespace std;

const int MAX_BUFFER_SIZE = 12000000;

GLFWwindow* window = NULL;
vector<glm::vec4> points;
GLuint snowProgram, feedbackProgram;
GLuint vertexLocation;
GLuint vao, vbo, tbo;
GLfloat wind = 0.0f;

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


/*
* Event handler for mouse clicks
*
* When a mouse button is being held down, it enables ScreenLock
*/
void MouseButton(GLFWwindow * window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		ScreenLock = true;
    }
	if (action == GLFW_RELEASE)
    {
		ScreenLock = false;
        glfwSetCursorPos(window, ScreenWidth/2.0, ScreenHeight/2.0);
    }
}

/*
* Handler for keeping track of mouse position
*/
void CursorPos(GLFWwindow * window, double xpos, double ypos)
{
	//Check if holding down mouse button
	if (ScreenLock)
	{
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

  glUniform1f(glGetUniformLocation(snowProgram, "wind"), wind);

  glDrawArrays(GL_POINTS, 0, (int)points.size());

  glBindVertexArray(0);
  glUseProgram(0);
  
  glfwSwapBuffers(window);
}

/*
 * Generates a point within a [-1,1] cube and stuffs it in the points array.
 */
void GeneratePoint()
{
    float x = 1.0f, y = 1.0f, z = 1.0f, velocity;
    x -= (rand() % 200) / 100.0f;
    y -= (rand() % 200) / 100.0f;
    z -= (rand() % 200) / 100.0f;
    velocity = ((rand() % 10 + 9) / 20000.0f);
    points.push_back(glm::vec4(x, y, z, velocity));
}

void LoadPoints()
{
  //Create Points
  float x,y,z,velocity = 1.0f;
  /* initialize random seed: */
  srand (time(NULL));
  for (float k = -1; k <= 1; k+= 0.03f)
    for (float i = -1; i <= 1; i+= 0.03f)
      for (float j = -1; j <= 1; j+= 0.03f)
      {
		// Generate numbers in the range of 0.09 to 0.18, then offset by k/i for
		// final numbers in the range -0.91 to 1.18
        x = ((rand() % 10 + 9)/100.0f) + k; 
        y = ((rand() % 10 + 9)/100.0f) + i; 
		// Range is -0.94 to 1.15
        z = ((rand() % 10 + 6)/100.0f) + j;
        velocity = ((rand() % 10 + 9)/20000.0f); 
        points.push_back(glm::vec4(x,y,z,velocity));
      }
  
  particleCount = points.size();
  cout <<"Particle Count: " << particleCount << endl;
  
  //Attach to buffer and vao
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, MAX_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(glm::vec4), &points[0][0]);

  glBindVertexArray(0); 
}

/*
 * Checks to see if the particle count is where we want it, and adds/removes particles 
 * if necessary.
 */
void AdjustPoints()
{
    int difference = particleCount - points.size();

	// Early exit if no changes needed.
	if (difference == 0) return;

	int previous = points.size();


    if (difference < 0)
    {
        // More points than there should be, get rid of some.
        for (int i = 0; i > std::max(difference, -maxChangePerFrame); i--)
        {
            points.pop_back();
		}
    }
    else if (difference > 0)
    {
        // Not enough points, make more.
        for (int i = 0; i < std::min(difference, maxChangePerFrame); i++)
        {
            GeneratePoint();
		}
		// Buffer new data.
		glBindVertexArray(vao);
		//glBindBuffer(GL_ARRAY_BUFFER, tbo);
		//glBufferSubData(GL_ARRAY_BUFFER, previous, difference * sizeof(glm::vec4), &points[previous][0]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, previous, difference * sizeof(glm::vec4), &points[previous][0]);
	}
	cout << "Particle count: " << points.size() << endl;

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
  cameraDirection = glm::vec3(cos(cameraTheta) * sin(cameraPhi),
                                       sin(cameraTheta),
                                       cos(cameraTheta) * cos(cameraPhi));
  glm::vec3 cameraTarget = cameraPosition + cameraDirection; 
  cameraRight = glm::vec3( sin(cameraPhi - M_PI/2.0f),
                               0,
                               cos(cameraPhi - M_PI/2.0f));
  glm:: vec3 upVector = glm::cross( cameraRight, cameraDirection );
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
  ScreenWidth = 800;
  ScreenHeight = 800;
  window = glfwCreateWindow(ScreenWidth, ScreenHeight, "Falling Snow", NULL, NULL);
  if (!window) {
    glfwTerminate();
    printf("glfw window creation failed\n");
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);
  //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
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

  /*
  * Sets blending mode for snowflakes
  * TODO: configure it properly
  */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  while(!glfwWindowShouldClose(window))
  {
    /*
    * Polls for Mouse & Keyboard Events
    */
    glfwPollEvents();
    LoadMVP();
	AdjustPoints();

    /*
    * Draws scene to screen
    */
    Render();
    wind += 0.001;

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


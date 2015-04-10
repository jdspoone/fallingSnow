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

const int MAX_BUFFER_SIZE = 12000000;

GLFWwindow* window = NULL;
GLuint Frames = 0;
double Timer = glfwGetTime();
vector<glm::vec3> points;
GLuint snowProgram, feedbackProgram, backdropProgram, floorProgram;
GLuint vertexLocation;
GLuint vao, vbo, tbo, plane_vao, back_vbo, floor_vbo;
GLuint backTID, floorTID;

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
  if (key == GLFW_KEY_1 && action == GLFW_PRESS)
      key_one = !key_one;
  if (key == GLFW_KEY_2 && action == GLFW_PRESS)
      key_two = !key_two;
}


/*
* Draw Calls
*/
void Render()
{
  glClearColor(0.6f,0.6f,0.6f,0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //Draw Floor
  if (key_two)
  {
  glUseProgram(floorProgram);
  glBindVertexArray(plane_vao);

  glBindBuffer(GL_ARRAY_BUFFER, floor_vbo); 
  glEnableVertexAttribArray(0); //vertices location
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 5, 0);
  glEnableVertexAttribArray(1); //texture coords location
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 5, (void*)(sizeof(GL_FLOAT)*3));
  glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  //Draw Backdrop  
  if (key_one)
  {
  glUseProgram(backdropProgram);

  glBindVertexArray(plane_vao);
  glBindBuffer(GL_ARRAY_BUFFER, back_vbo); 
  
  glEnableVertexAttribArray(0); //vertices location
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 5, 0);
  glEnableVertexAttribArray(1); //texture coords location
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 5, (void*)(sizeof(GL_FLOAT)*3));
  
  GLuint texUnitLoc = glGetUniformLocation(backdropProgram, "texUnit"); 
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, backTID); 
  glUniform1i(backdropProgram, texUnitLoc);

  glDrawArrays(GL_TRIANGLES, 0, 6);
  }
  //Draw Snow
  glUseProgram(snowProgram);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
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
    points.push_back(glm::vec3(x, y, z));
}


void LoadPoints()
{
  //Create Points
  float x,y,z,velocity = 1.0f;
  /* initialize random seed: */
  srand ((unsigned int)time(NULL));
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
        points.push_back(glm::vec3(x,y,z));
      }
  
  particleCount = (unsigned int)points.size();
  cout <<"Particle Count: " << particleCount << endl;
  
  //Attach to buffer and vao
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, MAX_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(glm::vec3), &points[0][0]);

  glEnableVertexAttribArray(vertexLocation);
  glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindVertexArray(0); 
}


/*
 * Checks to see if the particle count is where we want it, and adds/removes particles 
 * if necessary.
 */
void AdjustPoints()
{
  int difference = particleCount - (int)points.size();

	// Early exit if no changes needed.
	if (difference == 0) return;

	int previous = (int)points.size();

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
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, previous * sizeof(glm::vec4), std::min(difference, maxChangePerFrame) * sizeof(glm::vec4), &points[previous][0]);

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
  glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), NULL, GL_DYNAMIC_READ);
      
  // Perform the feedback transform
  glBeginTransformFeedback(GL_POINTS);
  glDrawArrays(GL_POINTS, 0, (int)points.size());
  glEndTransformFeedback();
  glFlush();
			
  // Swap the 2 buffers
  std::swap(vbo, tbo);
	  
  glDisable(GL_RASTERIZER_DISCARD);

  glEnableVertexAttribArray(vertexLocation);
  glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
  
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


/*
* Loads a PNG from file, and adds info to texture
*/
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

//https://code.google.com/p/battlestar-tux/source/browse/procedural/simplexnoise.py
//Really should just find a decent SimplexNoise Library...
//Implemented this in Shaders/Floor/fragment.glsl
/*
GLfloat simplex2D(const GLfloat x, const GLfloat y)
{
   GLfloat total = 0.0f;
   GLfloat freq = 0.0f;
   GLfloat amplitude  = 1.0f; 
   GLfloat max_amplitude = 0.0f;
   GLuint octaves = 8; //8 or 16 is recommended for 2D simplex
   GLfloat persistence = 1.0; //Check out what this is supposed to be
   GLfloat scale = 1.0; //Check out what this is supposed to be

   GLuint perm[] = {
   151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
   8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
   35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
   134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
   55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
   18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
   250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
   189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
   172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
   228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
   107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
   138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,

   151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
   8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
   35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
   134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
   55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
   18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
   250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
   189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
   172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
   228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
   107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
   138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
   };

   vector<glm::vec3> grad3;
   grad3.push_back(glm::vec3(1, 1, 0));
   grad3.push_back(glm::vec3(-1, 1, 0));
   grad3.push_back(glm::vec3(1, -1, 0));
   grad3.push_back(glm::vec3(-1, -1, 0));
   grad3.push_back(glm::vec3(1, 0, 1));
   grad3.push_back(glm::vec3(-1, 0, 1));
   grad3.push_back(glm::vec3(1, 0, -1));
   grad3.push_back(glm::vec3(-1, 0, -1));
   grad3.push_back(glm::vec3(0, 1, 1));
   grad3.push_back(glm::vec3(0, -1, 1));
   grad3.push_back(glm::vec3(0, 1, -1));
   grad3.push_back(glm::vec3(0, -1, -1));
 
   for (int o = 0; o < octaves; o++)
   {
      //==== Begin Noise 2D ======
      GLfloat noise = 0.0;
      GLfloat n0 = 0.0, n1 = 0.0, n2 = 0.0;
      GLfloat X = x * freq;
      GLfloat Y = y * freq;

      GLfloat F2 = 0.5 * (sqrt(3.0) - 1.0);
      GLfloat s = (X + Y) * F2;
      GLfloat i = GLint(X + s);
      GLfloat j = GLint(Y + s);

      GLfloat G2 = (3.0 - sqrt(3.0) ) / 6.0;
      GLfloat t = GLfloat(i + j) * G2;
      GLfloat X0 = i - t;
      GLfloat Y0 = j - t;
      GLfloat x0 = X - X0;
      GLfloat y0 = Y - Y0;

      GLuint i1 = 0, j1 = 0;
      if (x0 > y0)
      {
          i1 = 1;
          j1 = 0;
      }
      else
      {
          i1 = 0;
          j1 = 1;
      }
      
      GLfloat x1 = x0 - i1 + G2;
      GLfloat y1 = y0 - j1 + G2;
      GLfloat x2 = x0 - 1.0 + 2.0 * G2;  
      GLfloat y2 = y0 - 1.0 + 2.0 * G2;
   
      GLint ii = GLint(i) & 255;
      GLint jj = GLint(j) & 255;
      GLint gi0 = perm[ii+perm[jj]] % 12;
      GLint gi1 = perm[ii+i1+perm[jj+j1]] % 12;
      GLint gi2 = perm[ii+1+perm[jj+1]] % 12; 

      GLfloat t0 = 0.5 - x0*x0 - y0*y0;
      if (t0 < 0)
      {
          n0 = 0.0;
      }
      else
      {
          t0 *= t0;
          n0 = t0 * t0 * (grad3[gi0].x * x0 + grad3[gi0].y * y0);
      }

      GLfloat t1 = 0.5 - x1*x1 - y1*y1;
      if (t1 < 0)
      {
         n1 = 0.0;
      }
      else
      {
        t1 *= t1;
        n1 = t1 * t1 * (grad3[gi1].x * x1 + grad3[gi1].y * y1);
      }

      GLfloat t2 = 0.5 - x2*x2-y2*y2;
      if (t2 < 0)
      {
        n2 = 0.0;
      }
      else
      {
        t2 *= t2;
        n2 = t2 * t2 * (grad3[gi2].x * x2 + grad3[gi2].y * y2);
      }
     
      noise = 70 * (n0 + n1 + n2);
 
      //==== End Noise 2D ========
   
      // Update Values
      total += noise * amplitude;
      freq *= 2.0;
      max_amplitude += amplitude;
      amplitude *= persistence; 

   }
   return (total / max_amplitude);
}
*/

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
  const GLchar* feedbackVaryings[] = { "nextPosition" };
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

  //Create VAO for planes (backdrops)
  glGenVertexArrays(1, &plane_vao);
  glBindVertexArray(plane_vao);

  glEnableVertexAttribArray(0); //vertices location
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 5, 0);
  glEnableVertexAttribArray(1); //texture coords location
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 5, (void*)(sizeof(GL_FLOAT)*3));

  // Setup Back Plane VBO
  glGenBuffers(1, &back_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, back_vbo); 
  //Draw back plane
   GLfloat vertexBackPlaneData[] = {   //Triangle A
                                   -0.1f, 0.1f, 1.0f, //v0
                                    0.0f, 0.0f,       //uv0
 
                                   -0.1f, -0.1f, 1.0f,//v1
                                    0.0f, 1.0f,       //uv1

                                    0.1f, 0.1f, 1.0f, //v2
                                    1.0f, 0.0f,       //uv2

                                    //Triangle B
                                    0.1f, 0.1f, 1.0f, //v3
                                    1.0f, 0.0f,       //uv3

                                   -0.1f, -0.1f, 1.0f,//v4
                                    0.0f, 1.0f,       //uv4

                                    0.1f, -0.1f, 1.0f,//v5
                                    1.0f, 1.0f        //uv6
													  			};
  glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*30 , &vertexBackPlaneData[0], GL_STATIC_DRAW);
  //glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind 

  //Load Texture for plane
  glGenTextures(1, &backTID);
  glBindTexture(GL_TEXTURE_2D, backTID);
  LoadTexture("Textures/cowcube.png", backdropProgram, backTID); //Credit: Etienne!

  // Setup Floor Plane VBO
  glGenBuffers(1, &floor_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, floor_vbo); 
  //Draw back plane
  GLfloat vertexFloorPlaneData[] = {   //Triangle A
                                   -1.0f, -0.1f, 1.0f, //v0
                                    0.0f, 0.0f,       //uv0
 
                                   -1.0f, -0.1f, -1.0f,//v1
                                    0.0f, 1.0f,       //uv1

                                    1.0f, -0.1f, 1.0f, //v2
                                    1.0f, 0.0f,       //uv2

                                    //Triangle B
                                    1.0f, -0.1f, 1.0f, //v3
                                    1.0f, 0.0f,       //uv3

                                   -1.0f, -0.1f, -1.0f,//v4
                                    0.0f, 1.0f,       //uv4

                                    1.0f, -0.1f, -1.0f,//v5
                                    1.0f, 1.0f        //uv6
													  			};
  glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*30 , &vertexFloorPlaneData[0], GL_STATIC_DRAW);
  //glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind 

  //glGenTextures(1, &floorTID);
  //glBindTexture(GL_TEXTURE_2D, floorTID);
  //Generate 2D texture using simplex2D or perlin2D or something
  
  
  glBindVertexArray(0);

  glEnable(GL_PROGRAM_POINT_SIZE); //Will remove after geometry shader is implemented, maybe
  glEnable(GL_DEPTH_TEST);
}

/*
* Sets window title to FPS
*/
void FPS()
{
	double elapsed = glfwGetTime() - Timer;
	if (elapsed > 1.0)
	{
		char title[32];
		sprintf(title,"Falling Snow, FPS: %0.2f",Frames/elapsed);
		glfwSetWindowTitle(window,title);
		Timer = glfwGetTime();
		Frames = 0;
	}
    else
    {
        Frames++;
    }
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
    //Update FPS
    FPS();
   // getchar();
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
  glDeleteProgram(backdropProgram);
  glDeleteProgram(floorProgram);
  glDeleteProgram(feedbackProgram);
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}


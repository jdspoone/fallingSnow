#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS //In GLM taking degrees as params is deprecated
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <random>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <time.h>
#include "lodepng.h" //Credit: http://lodev.org/lodepng/

#ifdef _WIN32 
#define M_PI 3.14159265358979323846f
#define RESIZE 0
#elif __APPLE__
#define RESIZE 1
#endif



using namespace std;

GLFWwindow* window = NULL;
GLuint Frames = 0;
double Timer = glfwGetTime();
glm::mat4 MVP;
vector<glm::vec3> positions;
vector<glm::vec3> velocities;
vector<GLfloat> angles;
GLuint snowProgram, feedbackProgram, sceneProgram;
GLuint renderPosition, renderVelocity, renderAngle;
GLuint feedbackPosition, feedbackVelocity, feedbackAngle;
GLuint snowVAO, floorVAO;
GLuint positionVBO[2];
GLuint velocityVBO[2];
GLuint angleVBO[2];
GLuint floorTID;

unsigned int iteration = 0;

//====== Camera Settings =======
glm::vec3 cameraPosition = glm::vec3(0.1f); //initial starting position
glm::vec3 cameraDirection;
glm::vec3 cameraRight; 
GLfloat camera_step = 0.01f;
GLfloat cameraTheta, cameraPhi = 0.0f;

bool ScreenLock = false;
int ScreenWidth, ScreenHeight;

unsigned int particleCount = 300000;
unsigned int particleStep = 30000;
unsigned int maxChangePerFrame = 1000;
unsigned int maxParticles = 10000000;

// Wind stuff
GLuint windTex;
const int windTexSize = 256;
GLfloat wind[windTexSize][windTexSize][windTexSize][3];
GLfloat windCopy[windTexSize][windTexSize][windTexSize][3];

//Scene Toggles
bool key_one = false;
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
    else
        printf("Could not open %s shader file: %s\n", typeName, path);
  
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
    if (logLength > 0)
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
    std::vector<char> errorMessage(max(logLength, int(1)));
    glGetProgramInfoLog(programID, logLength, NULL, &errorMessage[0]);
    if (logLength > 0)
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
    std::vector<char> errorMessage(max(logLength, int(1)));
    glGetProgramInfoLog(programID, logLength, NULL, &errorMessage[0]);
    if (logLength > 0)
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
    std::vector<char> errorMessage(max(logLength, int(1)));
    glGetProgramInfoLog(programID, logLength, NULL, &errorMessage[0]);
    if (logLength > 0)
    fprintf(stdout, "Linking Check: %s\n", &errorMessage[0]);

    glDeleteShader(vertexID);
    glDeleteShader(fragmentID);

    return programID;
}


#pragma mark - OpenGL

// Loads a PNG from file, and adds info to texture
void LoadTexture(const char* filename, GLuint textureID, GLuint shaderID)
{
  // Load Image
    vector<unsigned char> image; //the raw pixels
    unsigned width, height;

  // Decode
  unsigned error = lodepng::decode(image, width, height, filename);
    cout<< filename <<" >> height: "<<height<<", width: "<<width<<endl;

  // If there's an error, display it
  if(error)
    cout << "decoder error " << error << ": " << lodepng_error_text(error) << endl;
    
    // Bind
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Load image into texture
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

    // For sampling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glBindTexture(GL_TEXTURE_2D, 0);
}


class GLobject
{
public:
    GLobject(glm::vec3 p, glm::vec3 n, glm::vec3 t)
    {
        position = p;
        normal = n;
        texture = t; //UV + Alpha
    }
    glm::vec3 position, normal, texture;
}; 


//Creates a VAO from GLobject data
GLuint CreateVAO3( vector < GLobject > data, GLuint shader)
{
    GLuint vertex_location = glGetAttribLocation(shader, "position");
    GLuint normal_location = glGetAttribLocation(shader, "normal");
    GLuint texture_location = glGetAttribLocation(shader, "texture");

    GLuint vertexArrayID;

    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLobject) * data.size(), &data[0], GL_STATIC_DRAW);    

    glEnableVertexAttribArray(vertex_location);
    glVertexAttribPointer(vertex_location,3,GL_FLOAT,GL_FALSE,sizeof(GLobject),(void*)0);

    glEnableVertexAttribArray(normal_location);
    glVertexAttribPointer(normal_location,3,GL_FLOAT,GL_FALSE,sizeof(GLobject),(GLvoid *)sizeof(glm::vec3));

    glEnableVertexAttribArray(texture_location);
    glVertexAttribPointer(texture_location,3,GL_FLOAT,GL_FALSE,sizeof(GLobject),(GLvoid *)(sizeof(glm::vec3)*2));

    glBindVertexArray(0);

    return vertexArrayID;
}


void LoadScenery()
{
  //Load Shader
  sceneProgram = loadShadersVF("Shaders/Scene/vertex.glsl", "Shaders/Scene/fragment.glsl");

  //Load a Texture
  glGenTextures(1, &floorTID); //load back texture ID
  glBindTexture(GL_TEXTURE_2D, floorTID);
  LoadTexture("Textures/cowcube.png", sceneProgram, floorTID); //Credit: Etienne!

  //Build Floor
  vector<GLobject> floor;
  glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);

  GLobject P0 = GLobject(glm::vec3(-1.0f, 0.0f, 1.0f),   //vertex
                         normal,                         //normal
                         glm::vec3(0.0f, 0.0f, 1.0f));   //tex coords + alpha

  GLobject P1 = GLobject(glm::vec3(-1.0f, 0.0f, -1.0f),   
                         normal,                         
                         glm::vec3(0.0f, 1.0f, 1.0f));   

  GLobject P2 = GLobject(glm::vec3(1.0f, 0.0f, 1.0f),   
                         normal,                         
                         glm::vec3(1.0f, 0.0f, 1.0f));  
 
  GLobject P3 = GLobject(glm::vec3(1.0f, 0.0f, 1.0f),   
                         normal,                         
                         glm::vec3(1.0f, 0.0f, 1.0f));   

  GLobject P4 = GLobject(glm::vec3(-1.0f, 0.0f, -1.0f),   
                         normal,                         
                         glm::vec3(0.0f, 1.0f, 1.0f));   

  GLobject P5 = GLobject(glm::vec3(1.0f, 0.0f, -1.0f),   
                         normal,                         
                         glm::vec3(1.0f, 1.0f, 1.0f));   

  floor.push_back(P0);
  floor.push_back(P1);
  floor.push_back(P2);
  floor.push_back(P3);
  floor.push_back(P4);
  floor.push_back(P5);

  floorVAO = CreateVAO3(floor, sceneProgram);
}


void setupRenderingContext()
{
  //Load vertex, geomtry, and fragment shaders for snow
  snowProgram = loadShadersVGF("Shaders/Snow/vertex.glsl", "Shaders/Snow/geometry.glsl", "Shaders/Snow/fragment.glsl");
 
  //Load vertex shader to preform feedback transformations on
  feedbackProgram = loadFeedbackShader("Shaders/Feedback/vertex.glsl");
  const GLchar* feedbackVaryings[] = { "nextPosition", "nextVelocity", "nextAngle" };
  glTransformFeedbackVaryings(feedbackProgram, 3, feedbackVaryings, GL_SEPARATE_ATTRIBS);
  glLinkProgram(feedbackProgram);

  // Attribute bindings
  renderPosition = glGetAttribLocation(snowProgram, "position");
  renderVelocity = glGetAttribLocation(snowProgram, "velocity");
  renderAngle = glGetAttribLocation(snowProgram, "angle");

  feedbackPosition = glGetAttribLocation(feedbackProgram, "previousPosition");
  feedbackVelocity = glGetAttribLocation(feedbackProgram, "previousVelocity");
  feedbackAngle = glGetAttribLocation(feedbackProgram, "previousAngle");

  // Generate our vertex array object
  glGenVertexArrays(1, &snowVAO);

  // Generate our vertex buffer objects
  glGenBuffers(2, positionVBO);
  glGenBuffers(2, velocityVBO);
  glGenBuffers(2, angleVBO);
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  // Create uniforms for the unit equilateral triangle, saves work in the geometry shader
  glUseProgram(snowProgram);
  
  glm::vec4 firstTriangleVertex = glm::vec4(cos(0), sin(0), 0.0, 0.0);
  glm::vec4 secondTriangleVertex = glm::vec4(cos(120 * M_PI / 180), sin(120 * M_PI / 180), 0.0, 0.0);
  glm::vec4 thirdTriangleVertex = glm::vec4(cos(240 * M_PI / 180), sin(240 * M_PI / 180), 0.0, 0.0);

  GLuint firstTriangleVertexID = glGetUniformLocation(snowProgram, "firstTriangleVertex");
  glUniform4fv(firstTriangleVertexID, 1, &firstTriangleVertex[0]);
  GLuint secondTriangleVertexID = glGetUniformLocation(snowProgram, "secondTriangleVertex");
  glUniform4fv(secondTriangleVertexID, 1, &secondTriangleVertex[0]);
  GLuint thirdTriangleVertexID = glGetUniformLocation(snowProgram, "thirdTriangleVertex");
  glUniform4fv(thirdTriangleVertexID, 1, &thirdTriangleVertex[0]);
  
  glUseProgram(0);
}

/*
Generates random noise in the provided array.
*/
void GenerateNoise()
{
    srand((unsigned int)time(NULL));
    for (int x = 0; x < windTexSize; x++)
    {
        for (int y = 0; y < windTexSize; y++)
        {
            for (int z = 0; z < windTexSize; z++)
            {
                // Initialize wind texture to random values.
                wind[x][y][z][0] = (rand() / (float)RAND_MAX) * -1.0f;
                wind[x][y][z][1] = (rand() / (float)RAND_MAX) / 2.0f;
                wind[x][y][z][2] = (rand() / (float)RAND_MAX) - 0.5f;
            }
        }
    }
    cout << "Noise generated in wind texture." << endl;
}

/*
Smooths the wind field out when zooming in on the texture.
*/
float SmoothNoise(float x, float y, float z, int index)
{
    // Get the fractional portion to figure out where we are between the values
    float fractX = x - int(x);
    float fractY = y - int(y);
    float fractZ = z - int(z);

    // Wrap around
    int x1 = int(x);
    int y1 = int(y);
    int z1 = int(z);
    int x2 = x1 == 0 ? windTexSize - 1 : x1 - 1;
    int y2 = y1 == 0 ? windTexSize - 1 : y1 - 1;
    int z2 = z1 == 0 ? windTexSize - 1 : z1 - 1;

    // Interpolate between the values;
    float value = 0.0f;
    value += fractX * fractY * fractZ * wind[x1][y1][z1][index];
    value += fractX * (1 - fractY) * fractZ * wind[x1][y2][z1][index];
    value += (1 - fractX) * fractY * fractZ * wind[x2][y1][z1][index];
    value += (1 - fractX) * (1 - fractY) * fractZ * wind[x2][y2][z1][index];
    value += fractX * fractY * (1 - fractZ) * wind[x1][y1][z2][index];
    value += fractX * (1 - fractY) * (1 - fractZ) * wind[x1][y2][z2][index];
    value += (1 - fractX) * fractY * (1 - fractZ) * wind[x2][y1][z2][index];
    value += (1 - fractX) * (1 - fractY) * (1 - fractZ) * wind[x2][y2][z2][index];

    return value;
}

/*
Adds together several zoomed-in versions of the wind texture.
*/
void Turbulence(float size)
{
    float value = 0.0f, startSize = size;

    for (int x = 0; x < windTexSize; x++)
    {
        for (int y = 0; y < windTexSize; y++)
        {
            for (int z = 0; z < windTexSize; z++)
            {
                // Do this for each x,y,z value
                for (int i = 0; i < 3; i++)
                {
                    size = startSize;
                    value = 0.0f;
                    while (size >= 1)
                    {
                        value += SmoothNoise(x / size, y / size, z / size, i) * size;
                        size /= 2.0f;
                    }
                    windCopy[x][y][z][i] = value / (startSize * 2.0f);
                }
            }
        }
    }
    cout << "Turbulence added to wind texture." << endl;
}


/*
Sets up the wind texture for use.
*/
void SetupWind()
{
    glEnable(GL_TEXTURE_3D);

    glGenTextures(1, &windTex);

    GenerateNoise();
    Turbulence(4.0f);

    glBindTexture(GL_TEXTURE_3D, windTex);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, windTexSize, windTexSize, windTexSize, 
        0, GL_RGB, GL_FLOAT, &windCopy[0][0][0][0]);

    //For sampling
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_3D, 0);
}


void LoadPoints()
{
  //Create Points
  float x,y,z;
  /* initialize random seed: */
  srand ((unsigned int)time(NULL));

  float b = 1.0f;    //Boundary
  int nrolls = 100; //Number of passes
  
  std::default_random_engine generator;
  std::uniform_real_distribution<float> distribution(-b,b);
  for (int k = 0; k <= nrolls; ++k) {
    for (int i = 0; i <= nrolls; ++i) {
      for (int j = 0; j <= nrolls; ++j) {
        x = distribution(generator);
        y = distribution(generator);
        z = distribution(generator);
        y += 1.0f;    // y should be positive
        y /= 2.0f;    // drop back into range of [0..1]
 
        positions.push_back(glm::vec3(x,y,z));
        velocities.push_back(glm::vec3(0.0, 0.0001, 0.0));
        angles.push_back((float)fmod((float)rand(), 90));
      }
    }
   }

  particleCount = (unsigned int)positions.size();
  cout <<"Particle Count: " << particleCount << endl;
  
  // Allocate and initialize the position vertex buffer
  for (int i = 0; i < 2; ++i) {
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO[i]);
    glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(glm::vec3), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), &positions[0][0]);
  }
  
  // Allocate and initialize the velocity vertex buffer
  for (int i = 0; i < 2; ++i) {
    glBindBuffer(GL_ARRAY_BUFFER, velocityVBO[i]);
    glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(glm::vec3), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, velocities.size() * sizeof(glm::vec3), &velocities[0][0]);
  }
  
  // Allocate and initialize the angle vertex buffer
  for (int i = 0; i < 2; ++i) {
    glBindBuffer(GL_ARRAY_BUFFER, angleVBO[i]);
    glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(glm::vec3), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, angles.size() * sizeof(GLfloat), &angles[0]);
  }
}

/*
Check the number of particles, then add or delete particles as necessary.
*/
void AdjustNumPoints()
{
    if (particleCount > positions.size())
    {
        float x, y, z;
        unsigned int stop = std::min(maxChangePerFrame, (unsigned int)(particleCount - positions.size()));
        for (unsigned int i = 0; i < stop; i++)
        {
            x = (rand() / (float)RAND_MAX - 0.5f) * 2;
            z = (rand() / (float)RAND_MAX - 0.5f) * 2;
            y = 1.0f;
            positions.push_back(glm::vec3(x, y, z));
            velocities.push_back(glm::vec3(0.0f, 0.0001f, 0.0f));
            angles.push_back((float)fmod((float)rand(), 90));
        }
        // Buffer new data
        for (int i = 0; i < 2; ++i) {
            glBindBuffer(GL_ARRAY_BUFFER, positionVBO[i]);
            glBufferSubData(GL_ARRAY_BUFFER, (positions.size() - stop) * sizeof(glm::vec3), stop * sizeof(glm::vec3), &positions[positions.size() - stop][0]);
        }
        for (int i = 0; i < 2; ++i) {
            glBindBuffer(GL_ARRAY_BUFFER, velocityVBO[i]);
            glBufferSubData(GL_ARRAY_BUFFER, (velocities.size() - stop) * sizeof(glm::vec3), stop * sizeof(glm::vec3), &velocities[velocities.size() - stop][0]);
        }
        for (int i = 0; i < 2; ++i) {
            glBindBuffer(GL_ARRAY_BUFFER, angleVBO[i]);
            glBufferSubData(GL_ARRAY_BUFFER, (angles.size() - stop) * sizeof(GLfloat), stop * sizeof(GLfloat), &angles[angles.size() - stop]);
        }
        //cout << "New particle count: " << positions.size() << endl;
    }
    else if (particleCount < positions.size())
    {
        for (unsigned int i = particleCount; i < positions.size(); )
        {
            positions.pop_back();
            velocities.pop_back();
            angles.pop_back();
        }
        //cout << "New particle count: " << positions.size() << endl;
    }
}


void RenderScene()
{
  // Bind the shader program
  glUseProgram(sceneProgram);

  //Activate texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, floorTID);

  //Pass through camera
  GLuint MatrixID = glGetUniformLocation(sceneProgram, "MVP");
  glUniformMatrix4fv(MatrixID,  1, GL_FALSE, &MVP[0][0]);

  // Bind the VAO
  glBindVertexArray(floorVAO);
  
  // Render snowflakes to screen
  glDrawArrays(GL_TRIANGLES, 0, 6);

  // Unbind the VAO
  glBindVertexArray(0);
  
  // Unbind the shader program
  glUseProgram(0);
}


void RenderSnow() {
  // Bind the shader program
  glUseProgram(snowProgram);

  //Pass through camera
  GLuint MatrixID = glGetUniformLocation(snowProgram, "MVP");
  glUniformMatrix4fv(MatrixID,  1, GL_FALSE, &MVP[0][0]);

  // Bind the VAO
  glBindVertexArray(snowVAO);
  
  // Establish the necessary attribute bindings for rendering
  glBindBuffer(GL_ARRAY_BUFFER, positionVBO[iteration % 2]);
  glEnableVertexAttribArray(renderPosition);
  glVertexAttribPointer(renderPosition, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
  
  glBindBuffer(GL_ARRAY_BUFFER, velocityVBO[iteration % 2]);
  glEnableVertexAttribArray(renderVelocity);
  glVertexAttribPointer(renderVelocity, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);

  glBindBuffer(GL_ARRAY_BUFFER, angleVBO[iteration % 2]);
  glEnableVertexAttribArray(renderAngle);
  glVertexAttribPointer(renderAngle, 1, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
  
  // Render snowflakes to screen
  glDrawArrays(GL_POINTS, 0, (int)positions.size());

  // Disable the attributes used for rendering
  glDisableVertexAttribArray(renderPosition);
  glDisableVertexAttribArray(renderVelocity);
  glDisableVertexAttribArray(renderAngle);

  // Unbind the VAO
  glBindVertexArray(0);
  
  // Unbind the shader program
  glUseProgram(0);
}


void Render()
{
  glClearColor(0.6f,0.6f,0.6f,0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  RenderSnow();

  if (key_one)
      RenderScene();
  
  glfwSwapBuffers(window);
}


void Feedback()
{
  glEnable(GL_RASTERIZER_DISCARD);
  
  // Bind the transform feedback program
  glUseProgram(feedbackProgram);
  
  // Bind the VAO
  glBindVertexArray(snowVAO);

  //Activate texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_3D, windTex);

  // Establish the necessary attribute bindings for transform feedback
  glBindBuffer(GL_ARRAY_BUFFER, positionVBO[iteration % 2]);
  glEnableVertexAttribArray(feedbackPosition);
  glVertexAttribPointer(feedbackPosition, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
  
  glBindBuffer(GL_ARRAY_BUFFER, velocityVBO[iteration % 2]);
  glEnableVertexAttribArray(feedbackVelocity);
  glVertexAttribPointer(feedbackVelocity, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);

  glBindBuffer(GL_ARRAY_BUFFER, angleVBO[iteration % 2]);
  glEnableVertexAttribArray(feedbackAngle);
  glVertexAttribPointer(feedbackAngle, 1, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);

  // Bind the output VBOs
  glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, positionVBO[(iteration + 1) % 2]);
  glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velocityVBO[(iteration + 1) % 2]);
  glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, angleVBO[(iteration + 1) % 2]);

  glUniform1f(glGetUniformLocation(feedbackProgram, "numParticles"), (float)positions.size());

  // Perform the feedback transform
  glBeginTransformFeedback(GL_POINTS);
  glDrawArrays(GL_POINTS, 0, (int)positions.size());
  glEndTransformFeedback();
  glFlush();
  
  // Swap the various vertex buffers
  std::swap(positionVBO[0], positionVBO[1]);
  std::swap(velocityVBO[0], velocityVBO[1]);
  std::swap(angleVBO[0], angleVBO[1]);
  
  // Disable the attributes used in transform feedback
  glDisableVertexAttribArray(feedbackPosition);
  glDisableVertexAttribArray(feedbackVelocity);
  glDisableVertexAttribArray(feedbackAngle);

  // Unbind the VAO
  glBindVertexArray(0);
  
  // Unbind the transform feedback program
  glUseProgram(0);
  
  glDisable(GL_RASTERIZER_DISCARD);
}


void UpdateMVP()
{
  //Camera
  cameraDirection = glm::vec3(cos(cameraTheta) * sin(cameraPhi),
                                       sin(cameraTheta),
                                       cos(cameraTheta) * cos(cameraPhi));
  glm::vec3 cameraTarget = cameraPosition + cameraDirection; 
  //cout<<"Camera Target: "<<cameraTarget.x<<","<<cameraTarget.y<<","<<cameraTarget.z<<endl; 
 
  cameraRight = glm::vec3(sin(cameraPhi - M_PI/2.0f),
                           0,
                           cos(cameraPhi - M_PI/2.0f));

  glm:: vec3 upVector = glm::cross(cameraRight, cameraDirection);
  glm::mat4 CameraMatrix = glm::lookAt(cameraPosition, cameraTarget, upVector);

  //Projection
  float fovy = M_PI * 0.25f; //Radians,this is equivalent to 45 degrees
  float aspect = ScreenWidth/(float)ScreenHeight;
  float zNear = 0.0001f;
  float zFar = 100.0f;
  glm::mat4 ProjectionMatrix = glm::perspective(fovy, aspect, zNear, zFar);

  //Model
  //nothing to do here yet
  glm::mat4 ModelMatrix = glm::mat4(1.0f);  //Identity Matrix

  //MVP
  glm::mat4 CameraMVP = ProjectionMatrix * CameraMatrix * ModelMatrix;
  MVP = CameraMVP;

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
       cameraPhi   +=  0.00005f * (ScreenWidth/2.0f - (float)xpos);
       cameraTheta +=  0.00005f * (ScreenHeight/2.0f - (float)ypos);
    }
}


void ResizeWindow(GLFWwindow* window, int width, int height)
{
    if (RESIZE)
        glfwGetFramebufferSize(window, &width, &height); //High-res display

    //Keep 1:1 aspect ratio
    int minsize = std::min(width, height);
    //offsets
    int xoff = width - minsize;
    int yoff = height - minsize;
    
    width = minsize;
    height = minsize;
    ScreenWidth = width;
    ScreenHeight = height;

    glViewport((int)(xoff/2.0), (int)(yoff/2.0), width, height);
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
    particleCount = std::min(particleCount + particleStep, maxParticles);
  if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
      particleCount = particleCount < particleStep ? 0u : particleCount - particleStep;
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
  glfwSetWindowSizeCallback(window, ResizeWindow);

  // Initialize GLEW
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if(err != GLEW_OK){
    printf("glewInit failed\n");
    exit(EXIT_FAILURE);
  }
  
  setupRenderingContext();
  SetupWind();
  UpdateMVP();
  LoadPoints();
  LoadScenery();

  while(!glfwWindowShouldClose(window)) {
    FPS();
    glfwPollEvents();
    AdjustNumPoints();
    UpdateMVP();
    Render();
    Feedback();
  }

  //Cleanup 
  glDeleteBuffers(2, positionVBO);
  glDeleteBuffers(2, velocityVBO);
  glDeleteBuffers(2, angleVBO);
  glDeleteVertexArrays(1, &snowVAO);
  glDeleteProgram(snowProgram);
  glDeleteProgram(sceneProgram);
  glDeleteProgram(feedbackProgram);
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}


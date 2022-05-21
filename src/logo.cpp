#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define M_PI 3.14159265358979323846
typedef float (*easingFunc)(float val);

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

bool isPaused = false;
void toggleAnimation(bool *isPaused);
void pauseAnimation(bool *isPaused);
void playAnimation(bool *isPaused);

void setColor(int colorID, float r, float g, float b, float a);
void setWorldTransform(int transformID, glm::mat4 transform);

float getInterpolatedVal(float duration, float time, float initialVal, float finalVal, easingFunc easing);

float easeInOutCubic(float val);
float easeOutBounce(float val);
float easeInOutQuart(float val);
float linear(float val);
float easeInOutSine(float val);

class Box
{
private:
  float x, y, w, h;
  float angle, scale;
  GLfloat vertices[12];
  GLuint indices[6];
  GLuint VBO, VAO, EBO;
  glm::mat4 transform;

  void setVertices(GLfloat vertices[12])
  {
    for (int i = 0; i < 12; i++)
    {
      this->vertices[i] = vertices[i];
    }
  }

  void setIndices(GLuint indices[6])
  {
    for (int i = 0; i < 6; i++)
    {
      this->indices[i] = indices[i];
    }
  }

  void initBuffers()
  {
    glGenVertexArrays(1, &this->VAO);
    glBindVertexArray(this->VAO);

    glGenBuffers(1, &this->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertices), this->vertices, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &this->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(this->indices), this->indices, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
  }

public:
  Box(float w, float h)
  {
    this->w = w;
    this->h = h;

    this->x = 0.0f;
    this->y = 0.0f;
    this->angle = 0.0f;
    this->scale = 1.0f;

    GLfloat vertices[] = {
        -w / 2,
        h / 2,
        0.0f,
        w / 2,
        h / 2,
        0.0f,
        -w / 2,
        -h / 2,
        0.0f,
        w / 2,
        -h / 2,
        0.0f,
    };

    GLuint indices[] = {
        0, 1, 2,
        1, 2, 3};

    this->setVertices(vertices);
    this->setIndices(indices);

    this->initBuffers();

    this->resetTransform();
  }

  void draw(int colorID, float r, float g, float b, float a)
  {
    setColor(colorID, r, g, b, a);

    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  }

  void resetTransform()
  {
    this->transform = glm::mat4(1.0f);
  }

  glm::mat4 getTransform()
  {
    return this->transform;
  }

  float getAngle()
  {
    return this->angle;
  }

  float getScale()
  {
    return this->scale;
  }

  glm::vec2 getPosition()
  {
    return glm::vec2(this->x, this->y);
  }

  void setTransform(glm::mat4 transform)
  {
    this->transform = transform;
  }

  void setAngle(float angle)
  {
    this->angle = angle;
  }

  void setScale(float scale)
  {
    this->scale = scale;
  }

  void setPosition(float x, float y)
  {
    this->x = x;
    this->y = y;
  }

  void updateTransform(int transformID)
  {
    this->resetTransform();

    this->setTransform(glm::translate(this->transform, glm::vec3(this->x, this->y, 0.0f)));
    this->setTransform(glm::rotate(this->transform, glm::radians(this->angle), glm::vec3(0.0f, 0.0f, 1.0f)));
    this->setTransform(glm::scale(this->transform, glm::vec3(this->scale)));

    setWorldTransform(transformID, this->transform);
  }

  void updateOffsetRotation(int transformID)
  {
    this->resetTransform();

    this->setTransform(glm::rotate(this->transform, glm::radians(this->angle), glm::vec3(0.0f, 0.0f, 1.0f)));
    this->setTransform(glm::translate(this->transform, glm::vec3(this->x, this->y, 0.0f)));
    this->setTransform(glm::scale(this->transform, glm::vec3(this->scale)));

    setWorldTransform(transformID, this->transform);
  }
};

int main()
{
  // * GLobal variables
  int window_width = 500;
  int window_height = 500;

  float box_size = 0.2f;
  float gap = 0.01f;

  const char *vertexShaderSource = "#version 330 core\n"
                                   "layout (location = 0) in vec3 aPos;\n"
                                   "uniform mat4 transform;\n"
                                   "void main()\n"
                                   "{\n"
                                   " gl_Position = transform * vec4(aPos, 1.0f);\n"
                                   "}\0";

  const char *fragmentShaderSource = "#version 330 core\n"
                                     "out vec4 FragColor;\n"
                                     "uniform vec4 clr;\n"
                                     "void main()\n"
                                     "{\n"
                                     "FragColor = clr;\n"
                                     "}\n\0";

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(window_width, window_height, "Microsoft logo", NULL, NULL);
  if (window == NULL)
  {
    std::cout << "Error: Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Error: Failed to initialise GLAD" << std::endl;
    return -1;
  }

  float posOffset = gap + box_size / 2;

  Box box1(box_size, box_size);
  Box box2(box_size, box_size);
  Box box3(box_size, box_size);
  Box box4(box_size, box_size);

  GLuint vertexShader;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  GLuint fragmentShader;
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  GLuint shaderProgram;
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  glUseProgram(shaderProgram);
  unsigned int colorID = glGetUniformLocation(shaderProgram, "clr");
  unsigned int transformID = glGetUniformLocation(shaderProgram, "transform");

  glViewport(0, 0, window_width, window_height);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  float startTime = -1.0f,
        currTime = -1.0f,
        prevTime = 0.0f,
        elapsedTime = 0.0f,
        dt = 0.0f;

  while (!glfwWindowShouldClose(window))
  {
    processInput(window);

    if (isPaused == false)
    {
      float timestamp = (float)glfwGetTime();
      if (currTime < 0.0f)
      {
        startTime = timestamp;
      }
      currTime = timestamp;
      elapsedTime = currTime - startTime;
      dt = elapsedTime - prevTime;

      float t1, t2;

      glClearColor(0.07f, 0.07f, 0.07f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      // * Transition template
      // t1 = t2;
      // t2 += 1.0f;
      // if(elapsedTime <= t2 && elapsedTime >= t1){
      //   float val = getInterpolatedVal(t2 - t1, elapsedTime - t1, 0.0f, 45.0f, easeInOutCubic);
      // }

      t1 = 0.0f;
      t2 = 0.8f;
      if (elapsedTime <= t2 && elapsedTime >= t1)
      {
        box1.setPosition(-posOffset, posOffset);
        box2.setPosition(posOffset, posOffset);
        box3.setPosition(posOffset, -posOffset);
        box4.setPosition(-posOffset, -posOffset);

        float val1 = getInterpolatedVal(t2 - t1 - 0.075, elapsedTime - t1, 1.0f, 0.5f, easeOutBounce);
        float val2 = getInterpolatedVal(t2 - t1, elapsedTime - t1, 1.0f, 0.5f, easeOutBounce);
        float val3 = getInterpolatedVal(t2 - t1 - 0.05, elapsedTime - t1, 1.0f, 0.5f, easeOutBounce);
        float val4 = getInterpolatedVal(t2 - t1 - 0.025, elapsedTime - t1, 1.0f, 0.5f, easeOutBounce);

        box1.setScale(val1);
        box2.setScale(val2);
        box3.setScale(val3);
        box4.setScale(val4);

        box1.updateTransform(transformID);
        box1.draw(colorID, 0.95f, 0.11f, 0.11f, 1.0f);
        box2.updateTransform(transformID);
        box2.draw(colorID, 0.5f, 0.74f, 0.0f, 1.0f);
        box3.updateTransform(transformID);
        box3.draw(colorID, 0.0f, 0.65f, 0.94f, 1.0f);
        box4.updateTransform(transformID);
        box4.draw(colorID, 1.0f, 0.73f, 0.0f, 1.0f);
      }

      t1 = t2;
      t2 += 0.75f;
      if (elapsedTime <= t2 && elapsedTime >= t1)
      {
        float val1 = getInterpolatedVal(t2 - t1, elapsedTime - t1, 0.0f, -90.0f, easeInOutQuart);

        box1.setAngle(val1);
        box2.setAngle(val1);
        box3.setAngle(val1);
        box4.setAngle(val1);

        box1.updateOffsetRotation(transformID);
        box1.draw(colorID, 0.95f, 0.11f, 0.11f, 1.0f);
        box2.updateOffsetRotation(transformID);
        box2.draw(colorID, 0.5f, 0.74f, 0.0f, 1.0f);
        box3.updateOffsetRotation(transformID);
        box3.draw(colorID, 0.0f, 0.65f, 0.94f, 1.0f);
        box4.updateOffsetRotation(transformID);
        box4.draw(colorID, 1.0f, 0.73f, 0.0f, 1.0f);
      }

      t1 = t2;
      t2 += 0.75f;
      if (elapsedTime <= t2 && elapsedTime >= t1)
      {
        box1.setPosition(posOffset, posOffset);
        box2.setPosition(posOffset, -posOffset);
        box3.setPosition(-posOffset, -posOffset);
        box4.setPosition(-posOffset, posOffset);

        float val1 = getInterpolatedVal(t2 - t1, elapsedTime - t1, 0.5f, 1.0f, easeInOutQuart);
        float val2 = getInterpolatedVal(t2 - t1, elapsedTime - t1, -90.0f, -180.0f, easeInOutQuart);

        box1.setScale(val1);
        box2.setScale(val1);
        box3.setScale(val1);
        box4.setScale(val1);

        box1.setAngle(val2);
        box2.setAngle(val2);
        box3.setAngle(val2);
        box4.setAngle(val2);

        box1.updateTransform(transformID);
        box1.draw(colorID, 0.95f, 0.11f, 0.11f, 1.0f);
        box2.updateTransform(transformID);
        box2.draw(colorID, 0.5f, 0.74f, 0.0f, 1.0f);
        box3.updateTransform(transformID);
        box3.draw(colorID, 0.0f, 0.65f, 0.94f, 1.0f);
        box4.updateTransform(transformID);
        box4.draw(colorID, 1.0f, 0.73f, 0.0f, 1.0f);
      }

      t1 = t2;
      t2 += 0.75f;
      if (elapsedTime <= t2 && elapsedTime >= t1)
      {
        box1.setPosition(-posOffset, -posOffset);
        box2.setPosition(-posOffset, posOffset);
        box3.setPosition(posOffset, posOffset);
        box4.setPosition(posOffset, -posOffset);

        float val = getInterpolatedVal(t2 - t1, elapsedTime - t1, 180.0f, 90.0f, easeInOutQuart);

        box1.setAngle(val);
        box2.setAngle(val);
        box3.setAngle(val);
        box4.setAngle(val);

        box1.updateOffsetRotation(transformID);
        box1.draw(colorID, 0.95f, 0.11f, 0.11f, 1.0f);
        box2.updateOffsetRotation(transformID);
        box2.draw(colorID, 0.5f, 0.74f, 0.0f, 1.0f);
        box3.updateOffsetRotation(transformID);
        box3.draw(colorID, 0.0f, 0.65f, 0.94f, 1.0f);
        box4.updateOffsetRotation(transformID);
        box4.draw(colorID, 1.0f, 0.73f, 0.0f, 1.0f);
      }

      t1 = t2;
      t2 += 0.8f;
      if (elapsedTime <= t2 && elapsedTime >= t1)
      {
        box1.setPosition(posOffset, -posOffset);
        box2.setPosition(-posOffset, -posOffset);
        box3.setPosition(-posOffset, posOffset);
        box4.setPosition(posOffset, posOffset);

        float val1 = getInterpolatedVal(t2 - t1 - 0.025, elapsedTime - t1, 1.0f, 0.5f, easeOutBounce);
        float val2 = getInterpolatedVal(t2 - t1 - 0.05, elapsedTime - t1, 1.0f, 0.5f, easeOutBounce);
        float val3 = getInterpolatedVal(t2 - t1, elapsedTime - t1, 1.0f, 0.5f, easeOutBounce);
        float val4 = getInterpolatedVal(t2 - t1 - 0.075, elapsedTime - t1, 1.0f, 0.5f, easeOutBounce);

        box1.setScale(val1);
        box2.setScale(val2);
        box3.setScale(val3);
        box4.setScale(val4);

        box1.updateTransform(transformID);
        box1.draw(colorID, 0.95f, 0.11f, 0.11f, 1.0f);
        box2.updateTransform(transformID);
        box2.draw(colorID, 0.5f, 0.74f, 0.0f, 1.0f);
        box3.updateTransform(transformID);
        box3.draw(colorID, 0.0f, 0.65f, 0.94f, 1.0f);
        box4.updateTransform(transformID);
        box4.draw(colorID, 1.0f, 0.73f, 0.0f, 1.0f);
      }

      t1 = t2;
      t2 += 0.75f;
      if (elapsedTime <= t2 && elapsedTime >= t1)
      {
        float val = getInterpolatedVal(t2 - t1, elapsedTime - t1, 0.0f, -90.0f, easeInOutQuart);

        box1.setAngle(val);
        box2.setAngle(val);
        box3.setAngle(val);
        box4.setAngle(val);

        box1.updateOffsetRotation(transformID);
        box1.draw(colorID, 0.95f, 0.11f, 0.11f, 1.0f);
        box2.updateOffsetRotation(transformID);
        box2.draw(colorID, 0.5f, 0.74f, 0.0f, 1.0f);
        box3.updateOffsetRotation(transformID);
        box3.draw(colorID, 0.0f, 0.65f, 0.94f, 1.0f);
        box4.updateOffsetRotation(transformID);
        box4.draw(colorID, 1.0f, 0.73f, 0.0f, 1.0f);
      }

      t1 = t2;
      t2 += 0.75f;
      if (elapsedTime <= t2 && elapsedTime >= t1)
      {
        box1.setPosition(-posOffset, -posOffset);
        box2.setPosition(-posOffset, posOffset);
        box3.setPosition(posOffset, posOffset);
        box4.setPosition(posOffset, -posOffset);

        float val1 = getInterpolatedVal(t2 - t1, elapsedTime - t1, 0.5f, 1.0f, easeInOutQuart);
        float val2 = getInterpolatedVal(t2 - t1, elapsedTime - t1, -90.0f, -180.0f, easeInOutQuart);

        box1.setScale(val1);
        box2.setScale(val1);
        box3.setScale(val1);
        box4.setScale(val1);

        box1.setAngle(val2);
        box2.setAngle(val2);
        box3.setAngle(val2);
        box4.setAngle(val2);

        box1.updateTransform(transformID);
        box1.draw(colorID, 0.95f, 0.11f, 0.11f, 1.0f);
        box2.updateTransform(transformID);
        box2.draw(colorID, 0.5f, 0.74f, 0.0f, 1.0f);
        box3.updateTransform(transformID);
        box3.draw(colorID, 0.0f, 0.65f, 0.94f, 1.0f);
        box4.updateTransform(transformID);
        box4.draw(colorID, 1.0f, 0.73f, 0.0f, 1.0f);
      }

      t1 = t2;
      t2 += 0.75f;
      if (elapsedTime <= t2 && elapsedTime >= t1)
      {
        box1.setPosition(posOffset, posOffset);
        box2.setPosition(posOffset, -posOffset);
        box3.setPosition(-posOffset, -posOffset);
        box4.setPosition(-posOffset, posOffset);

        float val = getInterpolatedVal(t2 - t1, elapsedTime - t1, 180.0f, 90.0f, easeInOutQuart);

        box1.setAngle(val);
        box2.setAngle(val);
        box3.setAngle(val);
        box4.setAngle(val);

        box1.updateOffsetRotation(transformID);
        box1.draw(colorID, 0.95f, 0.11f, 0.11f, 1.0f);
        box2.updateOffsetRotation(transformID);
        box2.draw(colorID, 0.5f, 0.74f, 0.0f, 1.0f);
        box3.updateOffsetRotation(transformID);
        box3.draw(colorID, 0.0f, 0.65f, 0.94f, 1.0f);
        box4.updateOffsetRotation(transformID);
        box4.draw(colorID, 1.0f, 0.73f, 0.0f, 1.0f);
      }

      if (elapsedTime > t2)
      {
        currTime = -1.0f;
      }

      prevTime = elapsedTime;
    }
    else
    {
      glfwSetTime(elapsedTime + startTime);

      setWorldTransform(transformID, box1.getTransform());
      box1.draw(colorID, 0.95f, 0.11f, 0.11f, 1.0f);
      setWorldTransform(transformID, box2.getTransform());
      box2.draw(colorID, 0.5f, 0.74f, 0.0f, 1.0f);
      setWorldTransform(transformID, box3.getTransform());
      box3.draw(colorID, 0.0f, 0.65f, 0.94f, 1.0f);
      setWorldTransform(transformID, box4.getTransform());
      box4.draw(colorID, 1.0f, 0.73f, 0.0f, 1.0f);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
  {
    glfwSetWindowShouldClose(window, true);
  }

  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
  {
    pauseAnimation(&isPaused);
  }

  if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
  {
    playAnimation(&isPaused);
  }
}

void setColor(int colorID, float r, float g, float b, float a)
{
  glUniform4f(colorID, r, g, b, a);
}

void setWorldTransform(int transformID, glm::mat4 transform)
{
  glUniformMatrix4fv(transformID, 1, GL_FALSE, glm::value_ptr(transform));
}

void toggleAnimation(bool *isPaused)
{
  if (*isPaused == true)
  {
    *isPaused = false;
  }
  else
  {
    *isPaused = true;
  }
}

void pauseAnimation(bool *isPaused)
{
  *isPaused = true;
}

void playAnimation(bool *isPaused)
{
  *isPaused = false;
}

float getInterpolatedVal(float duration, float timeOffset, float initialVal, float finalVal, easingFunc easing)
{
  return initialVal + easing(timeOffset / duration) * (finalVal - initialVal);
}

float easeInOutCubic(float val)
{
  return val < 0.5f ? 4 * val * val * val : 1 - pow(-2 * val + 2, 3) / 2;
}

float easeOutBounce(float val)
{
  float n1 = 7.5625;
  float d1 = 2.75;

  if (val < 1 / d1)
  {
    return n1 * val * val;
  }
  else if (val < 2 / d1)
  {
    return n1 * (val -= 1.5 / d1) * val + 0.75;
  }
  else if (val < 2.5 / d1)
  {
    return n1 * (val -= 2.25 / d1) * val + 0.9375;
  }
  else
  {
    return n1 * (val -= 2.625 / d1) * val + 0.9843;
  }
}

float easeInOutQuart(float val)
{
  return val < 0.5 ? 8 * val * val * val * val : 1 - pow(-2 * val + 2, 4) / 2;
}

float linear(float val)
{
  return val;
}

float easeInOutSine(float val)
{
  return -(cos(M_PI * val) - 1) / 2;
}
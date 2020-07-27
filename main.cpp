#include <cmath>
#include <stdexcept>
#include <string>
//
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
//
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
//
#include <glm/glm.hpp>
//
#include <glm/ext.hpp>

using namespace gl;
using namespace std;

const struct {
  float x, y;
} vertices[] = {
    {-10.0f, -10.0f}, {10.0f, -10.0f}, {10.0f, 10.0f}, {-10.0f, 10.0f}};
const struct { unsigned int index[3]; } triangles[] = {{0, 1, 2}, {0, 2, 3}};

const char* vertex_shader_text =
    "#version 330\n"
    "layout (location = 0) in vec2 vPos;"
    "uniform mat4 MVP;"
    "out vec2 pos;"
    "void main(){"
    "  gl_Position = MVP * vec4(vPos.x, vPos.y, 0.0, 1.0);"
    "  pos = vPos;"
    "}";

const char* fragment_shader_text =
    "#version 330\n"
    "in vec2 pos;"
    "void main(){"
    "  vec2 c = pos;"
    "  vec2 z = vec2(0.0, 0.0);"
    "  int it = 0;"
    "  int max_it = 1000;"
    "  for (; (dot(z, z) < 4.0) && (it < max_it); ++it){"
    "    vec2 tmp = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y);"
    "    z = tmp + c;"
    "  }"
    "  float scale = -log(float(it + 1) / max_it) / log(max_it);"
    "  gl_FragColor = vec4(scale, scale, scale, 1.0);"
    "}";

GLFWwindow* window;
int width;
int height;
float ratio;
int update = 2;
GLuint vertex_buffer;
GLuint triangle_buffer;
GLuint vertex_array;
GLuint program;
GLint mvp_location;
glm::vec2 origin{-0.5f, 0.0f};
float yfov = 2.5f;
glm::vec2 mouse_pos;
glm::vec2 old_mouse_pos{};

void init_context();
void init_mesh_data();
void init_shader_program();
void resize();
void render();

int main(void) {
  init_context();
  init_mesh_data();
  init_shader_program();
  resize();
  while (!glfwWindowShouldClose(window)) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    old_mouse_pos = mouse_pos;
    mouse_pos = {xpos, ypos};

    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS) {
      const auto mouse_move = mouse_pos - old_mouse_pos;
      origin -= mouse_move * yfov / float(height);
      resize();
    }

    if (update > 0) {
      render();
      --update;
    }
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
}

void init_context() {
  glfwSetErrorCallback([](int error, const char* description) {
    throw runtime_error("GLFW Error " + to_string(error) + ": " + description);
  });

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(800, 450, "OpenGL GPGPU: Mandelbrot Set with GLSL",
                            NULL, NULL);
  glfwMakeContextCurrent(window);
  glbinding::initialize(glfwGetProcAddress);

  glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode,
                                int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GLFW_TRUE);
  });

  glfwSetFramebufferSizeCallback(
      window, [](GLFWwindow* window, int width, int height) { resize(); });

  glfwSetScrollCallback(window, [](GLFWwindow* window, double x, double y) {
    yfov *= exp(-0.1f * y);
    yfov = clamp(yfov, 1e-5f, 10.0f);
    resize();
  });
}

void init_shader_program() {
  auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
  glCompileShader(vertex_shader);

  auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
  glCompileShader(fragment_shader);

  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  mvp_location = glGetUniformLocation(program, "MVP");
  glUseProgram(program);
}

void init_mesh_data() {
  glGenVertexArrays(1, &vertex_array);
  glBindVertexArray(vertex_array);

  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), nullptr);
  glEnableVertexAttribArray(0);

  glGenBuffers(1, &triangle_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangle_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangles), triangles,
               GL_STATIC_DRAW);

  glBindVertexArray(vertex_array);
}

void resize() {
  glfwGetFramebufferSize(window, &width, &height);
  ratio = float(width) / height;
  glViewport(0, 0, width, height);
  glm::mat4 mvp{1.0f};
  mvp = glm::translate(mvp, glm::vec3(0.0f, 0.0f, -1.0f));
  mvp =
      glm::ortho(origin.x - 0.5f * yfov * ratio, origin.x + 0.5f * yfov * ratio,
                 origin.y + 0.5f * yfov, origin.y - 0.5f * yfov, 0.1f, 100.0f) *
      mvp;
  glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
  update = 2;
}

void render() {
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawElements(GL_TRIANGLES,
                 sizeof(triangles) / sizeof(triangles[0].index[0]),
                 GL_UNSIGNED_INT, 0);
}
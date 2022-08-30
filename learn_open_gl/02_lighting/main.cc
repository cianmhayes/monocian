
#include <iostream>

// Needs to be included before glfw3, for some reason clang-format is putting it
// after.
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <open_gl/camera.h>
#include <open_gl/shader.h>
#include <open_gl/window.h>
#include <resources/load_resource.h>

DECLARE_RESOURCE(common_vertex_glsl)
DECLARE_RESOURCE(light_source_frag_glsl)
DECLARE_RESOURCE(lit_object_frag_glsl)

uint32_t CreateCube() {
  float vertices[] = {
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.5f,  -0.5f, -0.5f,
      0.0f,  0.0f,  -1.0f, 0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
      0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, -0.5f, 0.5f,  -0.5f,
      0.0f,  0.0f,  -1.0f, -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f,

      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,
      0.0f,  0.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  -0.5f, 0.5f,  0.5f,
      0.0f,  0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,

      -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  -0.5f,
      -1.0f, 0.0f,  0.0f,  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
      -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  -0.5f, -0.5f, 0.5f,
      -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,

      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  -0.5f,
      1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
      0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, 0.5f,
      1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, -0.5f,
      0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
      0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  -0.5f, -0.5f, 0.5f,
      0.0f,  -1.0f, 0.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,

      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  -0.5f,
      0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,
      0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f};

  uint32_t vao;
  uint32_t vbo;

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // glBindBuffer(GL_ARRAY_BUFFER, 0);
  // glBindVertexArray(0);

  return vao;
}

int main() {
  Window window = Window(800, 600, "LearnOpenGL", true);

  if (!window.IsValid()) {
    std::cout << "Failed to create window";
    glfwTerminate();
    return -1;
  }

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD";
    glfwTerminate();
    return -1;
  }

  glViewport(0, 0, 800, 600);
  glEnable(GL_DEPTH_TEST);
  window.AddFramebufferSizeCallback(
      [](Window* w, int32_t width, int32_t height) {
        glViewport(0, 0, width, height);
      });

  auto cam = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f),
                                      glm::vec3(0.0f, 0.0f, -1.0f),
                                      glm::vec3(0.0f, 1.0f, 0.0f));

  // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  Camera* camera_ptr = cam.get();
  window.AddMouseMovementCallback(
      [&camera_ptr](Window* w, double x_position, double y_position) {
        camera_ptr->ProcessMouseInput(x_position, y_position);
      });

  Resource vertex_shader = LOAD_RESOURCE(common_vertex_glsl);
  Resource light_source_fragment_shader =
      LOAD_RESOURCE(light_source_frag_glsl);
  Resource lit_object_fragment_shader = LOAD_RESOURCE(lit_object_frag_glsl);
  auto lit_shader = std::make_unique<Shader>(
      vertex_shader.ToString(), lit_object_fragment_shader.ToString());
  if (!lit_shader->is_valid()) {
    std::cout << "Failed to compile lit shader program: "
              << lit_shader->error();
    glfwTerminate();
    return -1;
  }
  lit_shader->use();

  auto light_source_shader = std::make_unique<Shader>(
      vertex_shader.ToString(), light_source_fragment_shader.ToString());
  if (!light_source_shader->is_valid()) {
    std::cout << "Failed to compile light source shader program: "
              << light_source_shader->error();
    glfwTerminate();
    return -1;
  }
  light_source_shader->use();

  uint32_t cube = CreateCube();
  uint32_t light = CreateCube();
  glm::vec3 light_pos(1.2f, 1.0f, 2.0f);
  glm::mat4 light_transform(1.0f);
  light_transform = glm::translate(light_transform, light_pos);

  float delta_time = 0.0f;
  float last_frame = glfwGetTime();
  while (window) {
    if (window.IsKeyPressed(GLFW_KEY_ESCAPE))
      window.SetShouldClose(true);
    float current_frame = glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;
    float camera_speed = 2.0f * delta_time;
    cam->ProcessDirectionKeys(window.IsKeyPressed(GLFW_KEY_W),
                              window.IsKeyPressed(GLFW_KEY_S),
                              window.IsKeyPressed(GLFW_KEY_A),
                              window.IsKeyPressed(GLFW_KEY_D), delta_time);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    lit_shader->use();
    lit_shader->set_vec3f("object_color", 1.0f, 0.5f, 0.31f);
    lit_shader->set_vec3f("light_color", 1.0f, 1.0f, 1.0f);
    lit_shader->set_vec3f("light_position", light_pos.x, light_pos.y,
                          light_pos.z);
    glm::vec3 camera_pos = cam->GetPosition();
    lit_shader->set_vec3f("view_position", camera_pos.x, camera_pos.y,
                          camera_pos.z);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f),
                        glm::vec3(0.5f, 1.0f, 0.0f));
    lit_shader->set_mat4f("model", model);
    lit_shader->set_mat4f("projection", cam->GetProjectionMatrix());
    lit_shader->set_mat4f("view", cam->GetViewMatrix());
    glBindVertexArray(cube);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    light_source_shader->use();
    light_source_shader->set_mat4f("model", light_transform);
    light_source_shader->set_mat4f("projection", cam->GetProjectionMatrix());
    light_source_shader->set_mat4f("view", cam->GetViewMatrix());
    glBindVertexArray(light);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    window.SwapBuffers();
    glfwPollEvents();
  }
  return 0;
}
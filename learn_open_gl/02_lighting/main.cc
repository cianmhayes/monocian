
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
#include <open_gl/glfw_callback_manager.h>
#include <open_gl/shader.h>

const char* kVertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 a_pos;\n"
    "layout (location = 1) in vec3 a_normal;\n"
    "out vec3 normal_vector;\n"
    "out vec3 fragment_pos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "    fragment_pos = vec3(model * vec4(a_pos, 1.0));\n"
    "    normal_vector = mat3(transpose(inverse(model))) * a_normal;\n"
    "    gl_Position = projection * view * vec4(fragment_pos, 1.0);\n"
    "}\0";

const char* kFragmentShaderSource =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 normal_vector;\n"
    "in vec3 fragment_pos;\n"
    "uniform vec3 object_color;\n"
    "uniform vec3 light_color;\n"
    "uniform vec3 light_position;\n"
    "uniform vec3 view_position;\n"
    "void main()\n"
    "{\n"
    "    float ambient_strength = 0.1;\n"
    "    vec3 ambient = ambient_strength * light_color;\n"
    "    vec3 norm = normalize(normal_vector);\n"
    "    vec3 light_dir = normalize(light_position - fragment_pos);\n"
    "    float diff = max(dot(norm, light_dir), 0.0);\n"
    "    vec3 diffuse = diff * light_color;\n"
    "    float specular_strength = 0.5;\n"
    "    vec3 view_dir = normalize(view_position - fragment_pos);\n"
    "    vec3 reflect_dir = reflect(-light_dir, norm);\n"
    "    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);\n"
    "    vec3 specular = specular_strength * spec * light_color;\n"
    "    vec3 result = (ambient + diffuse + specular) * object_color;\n"
    "    FragColor = vec4(result, 1.0);\n"
    "}\0";

const char* kLightSourceFragmentShaderSource =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(1.0);\n"
    "}\0";

class ResizeViewportOnFramebufferSizeCallback : public FramebufferSizeCallback {
 public:
  ResizeViewportOnFramebufferSizeCallback(){};
  ~ResizeViewportOnFramebufferSizeCallback() override{};
  ResizeViewportOnFramebufferSizeCallback(
      const ResizeViewportOnFramebufferSizeCallback&) = delete;
  ResizeViewportOnFramebufferSizeCallback& operator=(
      const ResizeViewportOnFramebufferSizeCallback&) = delete;

  void Run(GLFWwindow* window, int width, int height) override {
    glViewport(0, 0, width, height);
  };
};

class MoveCameraCallback : public ProcessMouseInputCallback {
 public:
  MoveCameraCallback(Camera* cam) : cam_(cam){};
  ~MoveCameraCallback() override{};
  MoveCameraCallback(const MoveCameraCallback&) = delete;
  MoveCameraCallback& operator=(const MoveCameraCallback&) = delete;

  void Run(GLFWwindow* window, double x_pos, double y_pos) override {
    if (cam_) {
      cam_->ProcessMouseInput(window, x_pos, y_pos);
    }
  };

 private:
  Camera* cam_;
};

uint32_t CreateCube() {
float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

  uint32_t vao;
  uint32_t vbo;

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  //glBindBuffer(GL_ARRAY_BUFFER, 0);
  //glBindVertexArray(0);

  return vao;
}

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow* window =
      glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr);
  if (!window) {
    std::cout << "Failed to create window";
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD";
    glfwTerminate();
    return -1;
  }

  glViewport(0, 0, 800, 600);
  glEnable(GL_DEPTH_TEST);
  GlfwCallbackManager::GetInstance()->AddFramebufferSizeCallback(
      window, std::make_unique<ResizeViewportOnFramebufferSizeCallback>());

  auto cam = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f),
                                      glm::vec3(0.0f, 0.0f, -1.0f),
                                      glm::vec3(0.0f, 1.0f, 0.0f));

  auto process_input = std::make_unique<MoveCameraCallback>(cam.get());
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  GlfwCallbackManager::GetInstance()->AddProcessMouseInputCallback(window, std::move(process_input));

  auto lit_shader = std::make_unique<Shader>(
      std::string(kVertexShaderSource), std::string(kFragmentShaderSource));
  if (!lit_shader->is_valid()) {
    std::cout << "Failed to compile lit shader program: "
              << lit_shader->error();
    glfwTerminate();
    return -1;
  }
  lit_shader->use();


  auto light_source_shader =
      std::make_unique<Shader>(std::string(kVertexShaderSource),
                               std::string(kLightSourceFragmentShaderSource));
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
  while (!glfwWindowShouldClose(window)) {
    float current_frame = glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;
    cam->ProcessInput(window, delta_time);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    lit_shader->use();
    lit_shader->set_vec3f("object_color", 1.0f, 0.5f, 0.31f);
    lit_shader->set_vec3f("light_color", 1.0f, 1.0f, 1.0f);
    lit_shader->set_vec3f("light_position", light_pos.x, light_pos.y, light_pos.z);
    glm::vec3 camera_pos = cam->GetPosition();
    lit_shader->set_vec3f("view_position", camera_pos.x, camera_pos.y, camera_pos.z);
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

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
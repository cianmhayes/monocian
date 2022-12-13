
#include <iostream>

// Needs to be included before glfw3, for some reason clang-format is putting it
// after.
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ogl/camera.h>
#include <ogl/shader.h>
#include <ogl/simple_mesh.h>
#include <ogl/window.h>
#include <resources/load_resource.h>

DECLARE_RESOURCE(common_vertex_glsl)
DECLARE_RESOURCE(light_source_frag_glsl)
DECLARE_RESOURCE(lit_object_frag_glsl)
DECLARE_RESOURCE(container2_png)
DECLARE_RESOURCE(container2_specular_png)

bool LoadTextureAsset(const Resource& r,
                      uint32_t format,
                      bool flip,
                      ogl::SimpleMesh& mesh,
                      const std::string& texture_key) {
  int32_t width, height, channels;
  std::vector<uint8_t> proxy(r.begin(), r.end());
  uint8_t* data = stbi_load_from_memory(proxy.data(), r.size(), &width, &height,
                                        &channels, 0);

  if (!data)
    return false;

  mesh.SetTexture(texture_key, width, height, data);

  /*glGenTextures(1, texture_id);
  glBindTexture(GL_TEXTURE_2D, *texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
               GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);*/
  stbi_image_free(data);
  return true;
}

int main() {
  std::vector<float> vertices = {
      // positions          // normals           // texture coords
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  0.5f,  -0.5f,
      -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 0.0f,
      0.0f,  -1.0f, 1.0f,  1.0f,  0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
      1.0f,  1.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,

      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.5f,  -0.5f,
      0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,
      0.0f,  1.0f,  1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
      1.0f,  1.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

      -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,
      -0.5f, -1.0f, 0.0f,  0.0f,  1.0f,  1.0f,  -0.5f, -0.5f, -0.5f, -1.0f,
      0.0f,  0.0f,  0.0f,  1.0f,  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
      0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  0.0f,  0.0f,
      -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.0f,

      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,
      -0.5f, 1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 1.0f,
      0.0f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
      0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.5f,  -0.5f,
      -0.5f, 0.0f,  -1.0f, 0.0f,  1.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  0.0f,
      -1.0f, 0.0f,  1.0f,  0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
      1.0f,  0.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.0f,  0.0f,
      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f,  1.0f,

      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.5f,  0.5f,
      -0.5f, 0.0f,  1.0f,  0.0f,  1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,
      1.0f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
      1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f};
  std::vector<size_t> vertex_attribs = {3, 3, 2};
  ogl::Window window = ogl::Window(800, 600, "LearnOpenGL", true);

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
      [](ogl::Window* w, int32_t width, int32_t height) {
        glViewport(0, 0, width, height);
      });

  auto cam = std::make_unique<ogl::Camera>(glm::vec3(0.0f, 0.0f, 3.0f));

  window.SetMouseCursorMode(ogl::MouseCursorMode::kDisabled);
  ogl::Camera* camera_ptr = cam.get();
  window.AddMouseMovementCallback(
      [&camera_ptr](ogl::Window* w, double x_position, double y_position) {
        camera_ptr->ProcessMouseInput(x_position, y_position);
      });

  Resource vertex_shader = LOAD_RESOURCE(common_vertex_glsl);
  Resource light_source_fragment_shader = LOAD_RESOURCE(light_source_frag_glsl);
  Resource lit_object_fragment_shader = LOAD_RESOURCE(lit_object_frag_glsl);
  auto lit_shader = std::make_unique<ogl::Shader>(
      vertex_shader.ToString(), lit_object_fragment_shader.ToString());
  if (!lit_shader->IsValid()) {
    std::cout << "Failed to compile lit shader program: "
              << lit_shader->GetInitializationError();
    glfwTerminate();
    return -1;
  }
  lit_shader->Use();

  auto light_source_shader = std::make_unique<ogl::Shader>(
      vertex_shader.ToString(), light_source_fragment_shader.ToString());
  if (!light_source_shader->IsValid()) {
    std::cout << "Failed to compile light source shader program: "
              << light_source_shader->GetInitializationError();
    glfwTerminate();
    return -1;
  }
  light_source_shader->Use();

  ogl::SimpleMesh cube;
  ogl::SimpleMesh light;
  cube.SetVertices(vertices, vertex_attribs);
  light.SetVertices(vertices, vertex_attribs);

  Resource container_resource = LOAD_RESOURCE(container2_png);
  LoadTextureAsset(container_resource, GL_RGBA, false, cube, "diffuse");
  Resource container_specular_resource = LOAD_RESOURCE(container2_specular_png);
  LoadTextureAsset(container_specular_resource, GL_RGBA, false, cube, "specular");
  glm::vec3 light_pos(1.2f, 1.0f, 2.0f);
  glm::mat4 light_transform(1.0f);
  light_transform = glm::translate(light_transform, light_pos);

  while (window.FrameStart()) {
    if (window.IsKeyPressed(GLFW_KEY_ESCAPE)) {
      window.SetShouldClose(true);
      continue;
    }

    float camera_speed = 2.0f * window.GetLastFrameDuration();
    cam->ProcessDirectionKeys(window.IsKeyPressed(GLFW_KEY_W),
                              window.IsKeyPressed(GLFW_KEY_S),
                              window.IsKeyPressed(GLFW_KEY_A),
                              window.IsKeyPressed(GLFW_KEY_D), camera_speed);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    lit_shader->Use();
    lit_shader->SetVec3f("light.ambient", 0.2f, 0.2f, 0.2f);
    lit_shader->SetVec3f("light.diffuse", 0.5f, 0.5f, 0.5f);
    lit_shader->SetVec3f("light.specular", 1.0f, 1.0f, 1.0f);
    lit_shader->SetVec3f("light.position", light_pos.x, light_pos.y,
                         light_pos.z);
    glm::vec3 camera_pos = cam->GetPosition();
    lit_shader->SetVec3f("view_position", camera_pos.x, camera_pos.y,
                         camera_pos.z);
    lit_shader->SetVec3f("material.ambient", 0.135f, 0.2225f, 0.1575f);
    lit_shader->SetInt32("material.diffuse", 0);
    lit_shader->SetInt32("material.specular", 1);
    lit_shader->SetFloat("material.shininess", 128 * 0.1f);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cube.GetTextureHandle("diffuse"));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, cube.GetTextureHandle("specular"));

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f),
                        glm::vec3(0.5f, 1.0f, 0.0f));
    lit_shader->SetMat4f("model", model);
    lit_shader->SetMat4f("projection", cam->GetProjectionMatrix());
    lit_shader->SetMat4f("view", cam->GetViewMatrix());
    cube.Draw();

    light_source_shader->Use();
    light_source_shader->SetMat4f("model", light_transform);
    light_source_shader->SetMat4f("projection", cam->GetProjectionMatrix());
    light_source_shader->SetMat4f("view", cam->GetViewMatrix());
    light.Draw();
  }
  return 0;
}
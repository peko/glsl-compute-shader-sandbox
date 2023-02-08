#include <cstdio>
#include <iostream>

#include "glad/gl.h"
//
#include "GLFW/glfw3.h"
//
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
//
#include "renderer.h"

Renderer* RENDERER;

static void glfwErrorCallback(int error, const char* description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void GLAPIENTRY debugMessageCallback([[maybe_unused]] GLenum source,
                                     GLenum type, [[maybe_unused]] GLuint id,
                                     GLenum severity,
                                     [[maybe_unused]] GLsizei length,
                                     const GLchar* message,
                                     [[maybe_unused]] const void* userParam) {
  if (type == GL_DEBUG_TYPE_ERROR) {
    spdlog::error("[GL] type = 0x{:x}, severity = 0x{:x}, message = {}", type,
                  severity, message);
  } else {
    spdlog::info("[GL] type = 0x{:x}, severity = 0x{:x}, message = {}", type,
                 severity, message);
  }
}

static void framebufferSizeCallback([[maybe_unused]] GLFWwindow* window,
                                    int width, int height) {
  RENDERER->setResolution(glm::uvec2(width, height));
}

void handleInput(GLFWwindow* window, const ImGuiIO& io) {
  // close window
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }

  // move camera
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    RENDERER->move(CameraMovement::FORWARD, io.DeltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    RENDERER->move(CameraMovement::LEFT, io.DeltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    RENDERER->move(CameraMovement::BACKWARD, io.DeltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    RENDERER->move(CameraMovement::RIGHT, io.DeltaTime);
  }

  // camera look around
  if (!io.WantCaptureMouse &&
      glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    RENDERER->lookAround(io.MouseDelta.x, io.MouseDelta.y);
  }
}

int main() {
  // init glfw
  glfwSetErrorCallback(glfwErrorCallback);
  if (!glfwInit()) {
    return -1;
  }

  // init window and OpenGL context
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_FALSE);
#else
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

  GLFWwindow* window = glfwCreateWindow(512, 512, "n-body", nullptr, nullptr);
  if (!window) {
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0);  // disable vsync

  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

  // init glad
  if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
    std::cerr << "failed to initialize OpenGL context" << std::endl;
    return -1;
  }

  glDebugMessageCallback(debugMessageCallback, 0);

  // init imgui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;

  // set imgui style
  ImGui::StyleColorsDark();
  // ImGuiStyle* style = &ImGui::GetStyle();
  // style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.08f);

  // init imgui backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 460 core");

  // init renderer
  RENDERER = new Renderer();

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    handleInput(window, io);

    // start imgui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    ImGui::ShowDemoWindow();

    ImGui::Begin("UI");
    {
      ImGui::Text("Framerate: %f", io.Framerate);

      ImGui::Separator();

      
      static int N_PARTICLES = RENDERER->getNumberOfParticles();
      if (ImGui::InputInt("N", &N_PARTICLES)) {
        N_PARTICLES = std::clamp(N_PARTICLES, 32, 32000);
        RENDERER->setNumberOfParticles(N_PARTICLES);
      }

      static float DT = RENDERER->getDt();
      if (ImGui::InputFloat("dt", &DT)) {
        DT = std::clamp(DT, 0.0f, 10000.0f);
        RENDERER->setDt(DT);
      }

      ImGui::Separator();
      ImGui::DragFloat("Box size", &RENDERER->box_size, 0.01, 0.01, 5.0, "%.2f");
      ImGui::DragFloat("Node size", &RENDERER->particle_size, 0.1, 0.1, 20.0, "%.2f");
      
      ImGui::Separator();
      ImGui::DragFloat("damping", &RENDERER->damping, 0.01, 0.01, 1.0, "%.2f");

      ImGui::Separator();
      ImGui::Text("Node attraction");
      float* K = RENDERER->K;
      ImGui::DragFloat4("A", &K[ 0], 0.001, -10.0, 10.0, "%.3f");
      ImGui::DragFloat4("B", &K[ 4], 0.001, -10.0, 10.0, "%.3f");
      ImGui::DragFloat4("C", &K[ 8], 0.001, -10.0, 10.0, "%.3f");
      ImGui::DragFloat4("D", &K[12], 0.001, -10.0, 10.0, "%.3f");
      if (ImGui::Button("Reset weights")) {
        for(int i=0; i<16; i++) K[i] = 0.0;
      }

      ImGui::Separator();
      ImGui::Text("Min radius");
      float* r = RENDERER->r;
      ImGui::DragFloat4("ra", &r[ 0], 0.001, 0.001, 1.0, "%.3f");
      ImGui::DragFloat4("rb", &r[ 4], 0.001, 0.001, 1.0, "%.3f");
      ImGui::DragFloat4("rc", &r[ 8], 0.001, 0.001, 1.0, "%.3f");
      ImGui::DragFloat4("rd", &r[12], 0.001, 0.001, 1.0, "%.3f");
      if (ImGui::Button("Reset min radiuses")) {
        for(int i=0; i<16; i++) r[i] = RENDERER->default_min_radius;
      }
      
      ImGui::Separator();
      ImGui::Text("Max radius");
      float* R = RENDERER->R;
      ImGui::DragFloat4("Ra", &R[ 0], 0.001, 0.001, 1.0, "%.3f");
      ImGui::DragFloat4("Rb", &R[ 4], 0.001, 0.001, 1.0, "%.3f");
      ImGui::DragFloat4("Rc", &R[ 8], 0.001, 0.001, 1.0, "%.3f");
      ImGui::DragFloat4("Rd", &R[12], 0.001, 0.001, 1.0, "%.3f");
      if (ImGui::Button("Reset max radiuses")) {
        for(int i=0; i<16; i++) R[i] = RENDERER->default_max_radius;
      }
 
      if (ImGui::Button("Reset particles")) {
        RENDERER->resetParticles();
      }
    }
    ImGui::End();

    // render
    RENDERER->render();

    // render imgui
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  // cleanup
  delete RENDERER;

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}

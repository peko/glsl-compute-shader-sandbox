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
      
      if (ImGui::Button("Reset particles")) {
        RENDERER->resetParticles();
      }
      
      ImGui::SameLine(); 
      if(ImGui::Checkbox("3D", &RENDERER->threeD)) {
        RENDERER->resetParticles();
      };
      if (RENDERER->threeD) {
        ImGui::SameLine(); 
        ImGui::SliderFloat("focal", &RENDERER->focal_distance,0.001, 2.0,"%.4f", 0);
      }


      // Attraction 
      ImGui::Separator();
      ImGui::Text("Node attraction");
      
      float* K = RENDERER->K;
      static float default_attraction = 0.0;
      static float min_max_a[] = {-0.01, 0.01};
      ImGui::SliderFloat2("min/max a", min_max_a, -10.0, 10.0, "%.4f", ImGuiSliderFlags_Logarithmic);
      ImGui::SliderFloat("Attr", &default_attraction, min_max_a[0], min_max_a[1], "%.4f", ImGuiSliderFlags_Logarithmic);
      if (ImGui::Button("Set attr")) {
        for(int i=0; i<36; i++) K[i] = default_attraction;
      }
      ImGui::SameLine();
      if (ImGui::Button("Random a")) {
        for(int i=0; i<36; i++) K[i] = (float(rand())/RAND_MAX)*(min_max_a[1]-min_max_a[0])+min_max_a[0];
      }

      ImGui::DragScalarN("A", ImGuiDataType_Float, &K[ 0], 6, 0.0001, &min_max_a[0], &min_max_a[1], "%.4f", 0);
      ImGui::DragScalarN("B", ImGuiDataType_Float, &K[ 6], 6, 0.0001, &min_max_a[0], &min_max_a[1], "%.4f", 0);
      ImGui::DragScalarN("C", ImGuiDataType_Float, &K[12], 6, 0.0001, &min_max_a[0], &min_max_a[1], "%.4f", 0);
      ImGui::DragScalarN("D", ImGuiDataType_Float, &K[18], 6, 0.0001, &min_max_a[0], &min_max_a[1], "%.4f", 0);
      ImGui::DragScalarN("E", ImGuiDataType_Float, &K[24], 6, 0.0001, &min_max_a[0], &min_max_a[1], "%.4f", 0);
      ImGui::DragScalarN("F", ImGuiDataType_Float, &K[30], 6, 0.0001, &min_max_a[0], &min_max_a[1], "%.4f", 0);
      
      
      // Min radius
      ImGui::Separator();
      ImGui::Text("Min radius");
      
      float* r = RENDERER->r;
      static float min_max_r[] = {0.01, 0.05};
      static float default_r = 0.01;
      ImGui::SliderFloat2("min/max r", min_max_r, 0.0001, 10.0, "%.4f", ImGuiSliderFlags_Logarithmic);
      ImGui::SliderFloat("r", &default_r, min_max_r[0], min_max_r[1], "%.4f", ImGuiSliderFlags_Logarithmic);
      if (ImGui::Button("Set r")) {
        for(int i=0; i<36; i++) r[i] = default_r;
      }
      ImGui::SameLine();
      if (ImGui::Button("Random r")) {
        for(int i=0; i<36; i++) r[i] = (float(rand())/RAND_MAX)*(min_max_r[1]-min_max_r[0])+min_max_r[0];
      }

      ImGui::DragScalarN("ra", ImGuiDataType_Float, &r[ 0], 6, 0.0001, &min_max_r[0], &min_max_r[1], "%.4f", 0);
      ImGui::DragScalarN("rb", ImGuiDataType_Float, &r[ 6], 6, 0.0001, &min_max_r[0], &min_max_r[1], "%.4f", 0);
      ImGui::DragScalarN("rc", ImGuiDataType_Float, &r[12], 6, 0.0001, &min_max_r[0], &min_max_r[1], "%.4f", 0);
      ImGui::DragScalarN("rd", ImGuiDataType_Float, &r[18], 6, 0.0001, &min_max_r[0], &min_max_r[1], "%.4f", 0);
      ImGui::DragScalarN("re", ImGuiDataType_Float, &r[24], 6, 0.0001, &min_max_r[0], &min_max_r[1], "%.4f", 0);
      ImGui::DragScalarN("rf", ImGuiDataType_Float, &r[30], 6, 0.0001, &min_max_r[0], &min_max_r[1], "%.4f", 0);
      

      // Max radius
      ImGui::Separator();
      ImGui::Text("Max radius");

      float* R = RENDERER->R;
      static float default_R = 0.050;
      static float min_max_R[] = {0.01, 0.05};
      ImGui::SliderFloat2("min/max R", min_max_R, 0.0001, 10.0, "%.4f", ImGuiSliderFlags_Logarithmic);
      ImGui::SliderFloat("R", &default_R, min_max_R[0], min_max_R[1], "%.4f", ImGuiSliderFlags_Logarithmic);
      if (ImGui::Button("Set R")) {
        for(int i=0; i<36; i++) R[i] = default_R;
      }
      ImGui::SameLine();
      if (ImGui::Button("Random R")) {
        for(int i=0; i<36; i++) R[i] = (float(rand())/RAND_MAX)*(min_max_R[1]-min_max_R[0])+min_max_R[0];
      }

      ImGui::DragScalarN("Ra", ImGuiDataType_Float, &R[ 0], 6, 0.0001, &min_max_R[0], &min_max_R[1], "%.4f", 0);
      ImGui::DragScalarN("Rb", ImGuiDataType_Float, &R[ 6], 6, 0.0001, &min_max_R[0], &min_max_R[1], "%.4f", 0);
      ImGui::DragScalarN("Rc", ImGuiDataType_Float, &R[12], 6, 0.0001, &min_max_R[0], &min_max_R[1], "%.4f", 0);
      ImGui::DragScalarN("Rd", ImGuiDataType_Float, &R[18], 6, 0.0001, &min_max_R[0], &min_max_R[1], "%.4f", 0);
      ImGui::DragScalarN("Re", ImGuiDataType_Float, &R[24], 6, 0.0001, &min_max_R[0], &min_max_R[1], "%.4f", 0);
      ImGui::DragScalarN("Rf", ImGuiDataType_Float, &R[30], 6, 0.0001, &min_max_R[0], &min_max_R[1], "%.4f", 0);
      

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

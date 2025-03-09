#include "game.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // Adjust the viewport based on the new width and height
    glViewport(0, 0, width, height);
}

neptune::Game::Game() {
    std::cout << "[INFO] Creating game...\n";
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE; // Ensure GLEW uses modern OpenGL techniques
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
}
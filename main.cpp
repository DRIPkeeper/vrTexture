#include "include/glad.h"
#define GLFW_INCLUDE_NONE
#include "include/linmath.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include <fstream>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

static const float CAMERA_SPEED = 0.1f;
static const float MOUSE_SENSITIVITY = 0.1f;
static const float FOV_MIN = 10.0f;
static const float FOV_MAX = 120.0f;

typedef struct {
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 world_up;
    float yaw;
    float pitch;
    float fov;
} Camera;

Camera camera;

void init_camera() {
    camera.position[0] = 0.0f;
    camera.position[1] = 0.0f;
    camera.position[2] = 3.0f;

    camera.front[0] = 0.0f;
    camera.front[1] = 0.0f;
    camera.front[2] = -1.0f;

    camera.up[0] = 0.0f;
    camera.up[1] = 1.0f;
    camera.up[2] = 0.0f;

    camera.world_up[0] = 0.0f;
    camera.world_up[1] = 1.0f;
    camera.world_up[2] = 0.0f;

    camera.yaw = -90.0f;
    camera.pitch = 0.0f;

    camera.fov = 45.0f;

    printf("Camera initialized.\n");
}

void check_shader_compile(GLuint shader) {
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << infoLog << std::endl;
    }
}

void check_program_link(GLuint program) {
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << infoLog << std::endl;
    }
}
void update_camera_vectors() {
    vec3 front;
    front[0] = cosf(camera.yaw * M_PI / 180.0f) * cosf(camera.pitch * M_PI / 180.0f);
    front[1] = sinf(camera.pitch * M_PI / 180.0f);
    front[2] = sinf(camera.yaw * M_PI / 180.0f) * cosf(camera.pitch * M_PI / 180.0f);
    vec3_norm(camera.front, front);

    vec3_mul_cross(camera.right, camera.front, camera.world_up);
    vec3_norm(camera.right, camera.right);
    vec3_mul_cross(camera.up, camera.right, camera.front);
    vec3_norm(camera.up, camera.up);
}

void calculate_view_matrix(mat4x4 view_matrix) {
    vec3 center;
    vec3_add(center, camera.position, camera.front);

    vec3 f, s, u;
    vec3_sub(f, center, camera.position);
    vec3_norm(f, f);

    vec3_mul_cross(s, f, camera.world_up);
    vec3_norm(s, s);

    vec3_mul_cross(u, s, f);

    mat4x4_identity(view_matrix);
    view_matrix[0][0] = s[0];
    view_matrix[0][1] = s[1];
    view_matrix[0][2] = s[2];
    view_matrix[1][0] = u[0];
    view_matrix[1][1] = u[1];
    view_matrix[1][2] = u[2];
    view_matrix[2][0] = -f[0];
    view_matrix[2][1] = -f[1];
    view_matrix[2][2] = -f[2];
    view_matrix[3][0] = -vec3_mul_inner(s, camera.position);
    view_matrix[3][1] = -vec3_mul_inner(u, camera.position);
    view_matrix[3][2] = vec3_mul_inner(f, camera.position);
}




void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    static double lastX = 400, lastY = 300;
    float xoffset = (float)(xpos - lastX) * MOUSE_SENSITIVITY;
    float yoffset = (float)(lastY - ypos) * MOUSE_SENSITIVITY;
    lastX = xpos;
    lastY = ypos;

    camera.yaw += xoffset;
    camera.pitch += yoffset;

    if (camera.pitch > 89.0f)
        camera.pitch = 89.0f;
    if (camera.pitch < -89.0f)
        camera.pitch = -89.0f;

    update_camera_vectors();
}

GLuint load_shader(const char* vertex_path, const char* fragment_path) {

    std::ifstream vertex_file(vertex_path);
    std::ifstream fragment_file(fragment_path);

    if (!vertex_file.is_open() || !fragment_file.is_open()) {
        std::cerr << "Error opening shader files\n";
        return 0;
    }

    std::string vertex_source((std::istreambuf_iterator<char>(vertex_file)), std::istreambuf_iterator<char>());
    std::string fragment_source((std::istreambuf_iterator<char>(fragment_file)), std::istreambuf_iterator<char>());

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertex_source_cstr = vertex_source.c_str();
    glShaderSource(vertex_shader, 1, &vertex_source_cstr, NULL);
    glCompileShader(vertex_shader);
    check_shader_compile(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragment_source_cstr = fragment_source.c_str();
    glShaderSource(fragment_shader, 1, &fragment_source_cstr, NULL);
    glCompileShader(fragment_shader);
    check_shader_compile(fragment_shader);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    check_program_link(shader_program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}

GLuint load_texture(const char* path) {
    // Загружаем изображение
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return 0;
    }


    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (nrChannels == 3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    else if (nrChannels == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    return texture;
}


GLuint create_shader_program() {
    const char* vertex_shader_source = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;

        uniform mat4 mvp;
        out vec3 fragPos;
        out vec3 normal;

        void main() {
            fragPos = aPos;
            normal = aNormal;
            gl_Position = mvp * vec4(aPos, 1.0);
        }
    )";

    const char* fragment_shader_source = R"(
    #version 330 core
    out vec4 FragColor;

    in vec3 fragPos;
    in vec3 normal;

    uniform vec4 lightColor; // Цвет света (куба)
    uniform vec3 lightPos;   // Позиция света (куба)
    uniform vec3 viewPos;    // Позиция камеры
    uniform vec4 objectColor; // Цвет объекта

    void main() {
        // Амбиентное освещение
        float ambientStrength = 0.1f;
        vec3 ambient = ambientStrength * lightColor.rgb;

        // Диффузное освещение
        vec3 lightDir = normalize(lightPos - fragPos);
        float diff = max(dot(normal, lightDir), 0.0f);
        vec3 diffuse = diff * lightColor.rgb;

        // Итоговый цвет с учетом амбиентного и диффузного освещения
        vec3 result = (ambient + diffuse) * objectColor.rgb;
        FragColor = vec4(result, 1.0f);
    }
)";



    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}
void process_input(GLFWwindow* window, vec3* lightPos, bool* isLightMode) {
    vec3 movement;

    // Переключение между режимами с помощью клавиши L
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        *isLightMode = !*isLightMode;  // Переключаем режим
        while (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS);  // Ожидаем отпускания клавиши
    }

    if (*isLightMode) {
        // Режим управления светом
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            (*lightPos)[2] -= CAMERA_SPEED;  // Перемещение света по оси -Z
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            (*lightPos)[2] += CAMERA_SPEED;  // Перемещение света по оси +Z
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            (*lightPos)[0] -= CAMERA_SPEED;  // Перемещение света по оси -X
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            (*lightPos)[0] += CAMERA_SPEED;  // Перемещение света по оси +X
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            (*lightPos)[1] += CAMERA_SPEED;  // Перемещение света по оси +Y
        }
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
            (*lightPos)[1] -= CAMERA_SPEED;  // Перемещение света по оси -Y
        }
    } else {
        // Режим управления камерой
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            vec3_scale(movement, camera.front, CAMERA_SPEED);
            vec3_add(camera.position, camera.position, movement);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            vec3_scale(movement, camera.front, CAMERA_SPEED);
            vec3_sub(camera.position, camera.position, movement);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            vec3_scale(movement, camera.right, CAMERA_SPEED);
            vec3_sub(camera.position, camera.position, movement);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            vec3_scale(movement, camera.right, CAMERA_SPEED);
            vec3_add(camera.position, camera.position, movement);
        }
        if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS && camera.fov < FOV_MAX)
            camera.fov += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS && camera.fov > FOV_MIN)
            camera.fov -= 1.0f;
    }
}

int main() {



    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Virtual Camera with Light", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }



    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_DEPTH_TEST);

    init_camera();
    update_camera_vectors();


    GLuint phong_shader = load_shader("D:/vr/zad3/shaders/shader.vert", "D:/vr/zad3/shaders/phong.frag");
    GLuint diffuse_shader = load_shader("D:/vr/zad3/shaders/shader.vert", "D:/vr/zad3/shaders/diffuse.frag");
    GLuint specular_shader = load_shader("D:/vr/zad3/shaders/shader.vert", "D:/vr/zad3/shaders/specular.frag");
    GLuint lambert_shader = load_shader("D:/vr/zad3/shaders/shader.vert", "D:/vr/zad3/shaders/lambert.frag");
    GLuint no_lighting_shader = load_shader("D:/vr/zad3/shaders/shader.vert", "D:/vr/zad3/shaders/no_lighting.frag");




   float vertices[] = {
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
            // Передняя грань
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
            // Левая грань
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
            // Правая грань
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
            // Нижняя грань
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
            // Верхняя грань
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
     };

    unsigned int lightVAO, lightVBO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);

    // Привязываем VAO (Vertex Array Object)
    glBindVertexArray(lightVAO);

    // Привязываем VBO (Vertex Buffer Object) для вершин
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Устанавливаем аттрибуты для координат вершин (позиции)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Устанавливаем аттрибуты для нормалей
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Устанавливаем аттрибуты для текстурных координат
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Отвязываем VAO
    glBindVertexArray(0);




    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glfwSetCursorPosCallback(window, cursor_position_callback);

    vec3 cube_positions[] = {
    {-4.0f, 0.0f, -5.0f},
    {-2.0f, 0.0f, -5.0f},
    { 0.0f, 0.0f, -5.0f},
    { 2.0f, 0.0f, -5.0f},
    { 4.0f, 0.0f, -5.0f}
    };

    // Массив цветов для кубов
    vec4 cube_colors[] = {
        {1.0f, 0.0f, 0.0f, 1.0f}, // Красный
        {0.0f, 1.0f, 0.0f, 1.0f}, // Зеленый
        {0.0f, 0.0f, 1.0f, 1.0f}, // Синий
        {1.0f, 1.0f, 0.0f, 1.0f}, // Желтый
        {1.0f, 0.0f, 1.0f, 1.0f}  // Розовый
    };

   while (!glfwWindowShouldClose(window)) {
       bool isLightMode = false;

       // Перемещаем вычисление lightPos в начало цикла
       mat4x4 model;
       mat4x4_identity(model);
       mat4x4_translate(model, 0.0f, 2.0f, -6.0f);  // Перемещение в точку (0, 2, -6)

       // Извлекаем lightPos из матрицы модели
       glm::vec3 lightPos;
       lightPos.x = model[3][0];
       lightPos.y = model[3][1];
       lightPos.z = model[3][2];
       vec3 lightPosVec3 = {lightPos.x, lightPos.y, lightPos.z};
       // Теперь вызываем process_input, передавая актуальное значение lightPos
       process_input(window, &lightPosVec3, &isLightMode);

       glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

       mat4x4 view, projection;
       calculate_view_matrix(view);
       mat4x4_perspective(projection, camera.fov, 800.0f / 600.0f, 0.1f, 100.0f);

       GLuint light_shader = load_shader("D:/vr/zad3/shaders/shader.vert", "D:/vr/zad3/shaders/light_shader.frag");

       // Активируем шейдер программы
       glUseProgram(light_shader);

       // Используем вычисленный lightPos
       glUniform3f(glGetUniformLocation(light_shader, "lightPos"), lightPos.x, lightPos.y, lightPos.z);


       // Передаем модельную матрицу в шейдер
       glUniformMatrix4fv(glGetUniformLocation(light_shader, "model"), 1, GL_FALSE, (const GLfloat*)model);

       // Передаем видовую матрицу в шейдер
       glUniformMatrix4fv(glGetUniformLocation(light_shader, "view"), 1, GL_FALSE, (const GLfloat*)view);

       // Передаем проекционную матрицу в шейдер
       glUniformMatrix4fv(glGetUniformLocation(light_shader, "projection"), 1, GL_FALSE, (const GLfloat*)projection);

       // Передаем цвет объекта, который будет использован в шейдере (это будет цвет куба света)
       glUniform3f(glGetUniformLocation(light_shader, "objectColor"), 1.0f, 1.0f, 1.0f);  // Белый цвет

       // Передаем цвет источника света (если используется в фрагментном шейдере для освещения)
       glUniform3f(glGetUniformLocation(light_shader, "lightColor"), 1.0f, 1.0f, 1.0f);  // Белый цвет света

       // Отрисовываем куб света
       glBindVertexArray(lightVAO);  // Привязываем VAO для куба
       glDrawArrays(GL_TRIANGLES, 0, 36);
    // Отрисовываем объекты сцены с различными шейдерами
    for (int i = 0; i <= 4; i++) {
        GLuint shader_to_use = 0;

        switch(i) {
            case 0: shader_to_use = phong_shader; break;
            case 1: shader_to_use = diffuse_shader; break;
            case 2: shader_to_use = specular_shader; break;
            case 3: shader_to_use = lambert_shader; break;
            case 4: shader_to_use = no_lighting_shader; break;
            default: shader_to_use = phong_shader; break;
        }
        glUseProgram(shader_to_use);
        GLuint texture = load_texture("D:/vr/zad3/wood-2045380_1280.jpg");
        // Используем выбранный шейдер

        glUniform1i(glGetUniformLocation(shader_to_use, "uTexture"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glm::vec3 lightPos;
        lightPos.x = model[3][0];  // Извлекаем значение из матрицы модели (сдвиг по оси X)
        lightPos.y = model[3][1];  // Извлекаем значение из матрицы модели (сдвиг по оси Y)
        lightPos.z = model[3][2];  // Извлекаем значение из матрицы модели (сдвиг по оси Z)
        vec3 lightColor = {1.0f, 1.0f, 1.0f}; // Цвет источника света (белый)

        // Передача данных в шейдер
        glUniform3fv(glGetUniformLocation(shader_to_use, "lightPos"), 1, &lightPos[0]);
        glUniform3fv(glGetUniformLocation(shader_to_use, "lightColor"), 1, (const GLfloat*)lightColor);
        glUniform3fv(glGetUniformLocation(shader_to_use, "viewPos"), 1, (const GLfloat*)camera.position);

        // Рассчитываем и передаем MVP матрицу для каждого куба
        mat4x4 model, mvp;
        mat4x4_identity(model);
        mat4x4_translate(model, cube_positions[i][0], cube_positions[i][1], cube_positions[i][2]);
        mat4x4_mul(mvp, projection, view); // Сначала умножаем projection на view
        mat4x4_mul(mvp, mvp, model); // Затем добавляем модельную матрицу

        GLuint mvp_location = glGetUniformLocation(shader_to_use, "MVP");
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);

        GLuint model_location = glGetUniformLocation(shader_to_use, "model");
        glUniformMatrix4fv(model_location, 1, GL_FALSE, (const GLfloat*)model);

        GLuint view_location = glGetUniformLocation(shader_to_use, "view");
        glUniformMatrix4fv(view_location, 1, GL_FALSE, (const GLfloat*)view);

        GLuint projection_location = glGetUniformLocation(shader_to_use, "projection");
        glUniformMatrix4fv(projection_location, 1, GL_FALSE, (const GLfloat*)projection);

        GLuint objectColor_location = glGetUniformLocation(shader_to_use, "objectColor");
        glUniform4fv(objectColor_location, 1, (const GLfloat*)cube_colors[i]);

        // Передаем атрибуты для текстурных координат, нормалей и вершин
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // Вершины
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // Нормали
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // Текстурные координаты
        glEnableVertexAttribArray(2);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glfwSwapBuffers(window);
    glfwPollEvents();
}


    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
#version 330 core

layout (location = 0) in vec3 aPos;       // Позиция
layout (location = 1) in vec3 aNormal;    // Нормаль
layout (location = 2) in vec2 aTexCoord;  // Текстурные координаты

uniform mat4 model;       // Модельная матрица
uniform mat4 view;        // Матрица вида
uniform mat4 projection;  // Матрица проекции

out vec3 fragPos;         // Позиция фрагмента в мировых координатах
out vec3 normal;          // Нормаль фрагмента
out vec2 TexCoord;        // Текстурные координаты

void main() {
    fragPos = vec3(model * vec4(aPos, 1.0));          // Трансформируем позицию вершины в мировые координаты
    normal = mat3(transpose(inverse(model))) * aNormal; // Преобразуем нормали в мировые координаты
    TexCoord = aTexCoord;                              // Передаем текстурные координаты

    gl_Position = projection * view * vec4(fragPos, 1.0); // Финальная позиция вершины
}

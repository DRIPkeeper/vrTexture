#version 330 core
out vec4 FragColor;

in vec3 fragPos;  // Позиция фрагмента
in vec3 normal;   // Нормаль фрагмента

uniform vec3 lightPos;   // Позиция источника света
uniform vec3 viewPos;    // Позиция камеры
uniform vec3 lightColor; // Цвет источника света

void main() {
    // Фоновое освещение
    vec3 ambient = 0.1f * lightColor;

    // Направление света
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 norm = normalize(normal);

    // Диффузное освещение
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = diff * lightColor;

    // Спекулярное освещение
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specular = spec * lightColor;

    // Суммируем освещение
    vec3 result = ambient + diffuse + specular;

    // Применяем синий цвет к итоговому результату освещения
    // Можно использовать синий цвет для эффекта, но также важно учитывать
    // взаимодействие с результатами освещения
    vec3 blueColor = vec3(0.0f, 0.0f, 1.0f); // Чистый синий цвет
    vec3 finalColor = result * blueColor; // Применяем синий оттенок к результату освещения

    // Отправляем итоговый цвет в буфер
    FragColor = vec4(finalColor, 1.0f);
}

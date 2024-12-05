#version 330 core

out vec4 FragColor;

in vec3 fragPos; // Позиция фрагмента
in vec3 normal;  // Нормаль фрагмента

uniform vec3 lightPos; // Позиция источника света
uniform vec3 viewPos;  // Позиция камеры
uniform vec3 lightColor; // Цвет источника света

void main() {
    // Освещенность от источника света
    vec3 ambient = 0.1f * lightColor; // Фоновое освещение

    // Вектор от фрагмента до источника света
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 norm = normalize(normal);  // Нормаль фрагмента
    vec3 viewDir = normalize(viewPos - fragPos);  // Вектор к камере
    vec3 reflectDir = reflect(-lightDir, norm); // Отраженный вектор

    // Спекулярное освещение (отраженный свет)
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specular = spec * lightColor;

    // Результат освещения (фоновое + спекулярное)
    vec3 result = ambient + specular;

    // Фиолетовый цвет, который мы добавляем к результату освещения
    vec3 purpleColor = vec3(0.5f, 0.0f, 0.5f); // RGB для фиолетового цвета

    // Умножаем результат освещения на фиолетовый цвет (окрашиваем модель в фиолетовый)
    vec3 finalColor = result + purpleColor;

    // Отображаем итоговый цвет
    FragColor = vec4(finalColor, 1.0f);
    //исправлен

}

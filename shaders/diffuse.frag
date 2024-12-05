#version 330 core

out vec4 FragColor;  // Итоговый цвет фрагмента

in vec3 fragPos;     // Позиция фрагмента в мировых координатах
in vec3 normal;      // Нормаль фрагмента

uniform vec3 lightPos;   // Позиция источника света
uniform vec3 lightColor; // Цвет источника света
uniform vec3 viewPos;    // Позиция наблюдателя

void main()
{
    // Нормализуем нормаль фрагмента
    vec3 norm = normalize(normal);

    // Направление от фрагмента к источнику света
    vec3 lightDir = normalize(lightPos - fragPos);

    // Диффузное освещение (обрабатываем угол между нормалью и направлением света)
    float diff = max(dot(norm, lightDir), 0.0);

    // Цвет объекта (красный)
    vec3 objectColor = vec3(1.0, 0.0, 0.0); // Красный

    // Фоновое освещение для более яркого эффекта (ambient)
    vec3 ambient = 0.1 * lightColor;  // Небольшое фоновое освещение

    // Применяем освещение к объекту: диффузное освещение + фоновое
    vec3 result = (ambient + diff * lightColor) * objectColor;

    // Выводим итоговый цвет фрагмента
    FragColor = vec4(result, 1.0);  // Альфа канал 1.0 (непрозрачный)
}

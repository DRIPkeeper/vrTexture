#version 330 core
out vec4 FragColor;

in vec3 normal;        // Нормаль фрагмента
in vec3 fragPos;       // Позиция фрагмента в мировых координатах
in vec2 TexCoord;      // Текстурные координаты, передаваемые из вершинного шейдера

uniform vec3 lightPos;    // Позиция источника света
uniform vec3 lightColor;  // Цвет света
uniform sampler2D uTexture; // Текстура объекта

void main() {
    // Амбиентное освещение
    vec3 ambient = 0.1f * lightColor;

    // Направление света
    vec3 lightDir = normalize(lightPos - fragPos);

    // Диффузное освещение
    float diff = max(dot(normal, lightDir), 0.0f);
    vec3 diffuse = diff * lightColor;

    // Цвет объекта из текстуры
    vec3 objectColor = texture(uTexture, TexCoord).rgb;

    // Итоговый цвет: комбинация амбиентного и диффузного освещения, умноженная на цвет из текстуры
    vec3 finalColor = objectColor * (ambient + diffuse);

    FragColor = vec4(finalColor, 1.0);  // Устанавливаем итоговый цвет с альфой равной 1.0
}

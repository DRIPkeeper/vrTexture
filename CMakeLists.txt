cmake_minimum_required(VERSION 3.10)
project(zad3)

set(CMAKE_CXX_STANDARD 20)

# Указываем путь к заголовочным файлам
include_directories(C:/mingw64/include)  # Путь к заголовочным файлам MinGW
include_directories(D:/vr/zad3/include)        # Путь к заголовочным файлам вашего проекта
include_directories(D:/vr/zad3/glm)  # Путь к заголовочным файлам GLM (проверьте этот путь)

# Указываем путь к библиотекам
link_directories(C:/mingw64/lib)  # Путь к библиотекам MinGW

# Создаем исполнимый файл
add_executable(zad3 main.cpp D:/vr/zad3/glad.c)

# Линковка с GLFW
target_link_libraries(zad3 glfw3)  # GLFW должен быть найден автоматически

# Копируем glfw3.dll в папку с исполнимым файлом после сборки
add_custom_command(TARGET zad3 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        C:/mingw64/lib/glfw3.dll $<TARGET_FILE_DIR:zad3>
)

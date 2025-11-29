// pr.cpp
#include "model.h"
#include "func.h"
#include "globals.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <cmath>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb-master/stb_image.h"

int WinWidth;
int WinHeight;

struct SimpleMesh {
    std::vector<glm::vec3> verts;
    std::vector<glm::vec3> cols;
    std::vector<glm::vec2> uvs;
    std::vector<GLuint> inds;
};

SimpleMesh make_box(glm::vec3 center, glm::vec3 size, glm::vec3 color, bool invert = false) {
    // создаём 8 вершин и индексы для 12 треугольников
    glm::vec3 hs = size * 0.5f;
    glm::vec3 v[8] = {
        center + glm::vec3(-hs.x, -hs.y, -hs.z),
        center + glm::vec3(hs.x, -hs.y, -hs.z),
        center + glm::vec3(hs.x,  hs.y, -hs.z),
        center + glm::vec3(-hs.x,  hs.y, -hs.z),

        center + glm::vec3(-hs.x, -hs.y,  hs.z),
        center + glm::vec3(hs.x, -hs.y,  hs.z),
        center + glm::vec3(hs.x,  hs.y,  hs.z),
        center + glm::vec3(-hs.x,  hs.y,  hs.z)
    };
    GLuint faceIndices[] = {
        0,1,2, 0,2,3, // back
        4,5,6, 4,6,7, // front
        8,9,10, 8,10,11, // left (we will remap)
    };
    SimpleMesh m;
    for (int i = 0; i < 8; i++) m.verts.push_back(v[i]);
    for (int i = 0; i < 8; i++) m.cols.push_back(color);

    // indices manual (6 faces)
    GLuint idxs[] = {
        0,1,2, 0,2,3,
        4,5,6, 4,6,7,
        0,4,7, 0,7,3,
        1,5,6, 1,6,2,
        3,2,6, 3,6,7,
        0,1,5, 0,5,4
    };
    for (auto v : idxs) m.inds.push_back(v);
    if (invert) {
        // перевернём порядок треугольников
        for (size_t i = 0; i < m.inds.size(); i += 3) {
            std::swap(m.inds[i + 1], m.inds[i + 2]);
        }
    }
    return m;
}

SimpleMesh make_textured_box(glm::vec3 center, glm::vec3 size) {
    glm::vec3 hs = size * 0.5f;
    glm::vec3 v[8] = {
        center + glm::vec3(-hs.x, -hs.y, -hs.z), // 0
        center + glm::vec3(hs.x, -hs.y, -hs.z),  // 1
        center + glm::vec3(hs.x,  hs.y, -hs.z),  // 2
        center + glm::vec3(-hs.x,  hs.y, -hs.z), // 3
        center + glm::vec3(-hs.x, -hs.y,  hs.z), // 4
        center + glm::vec3(hs.x, -hs.y,  hs.z),  // 5
        center + glm::vec3(hs.x,  hs.y,  hs.z),  // 6
        center + glm::vec3(-hs.x,  hs.y,  hs.z)  // 7
    };

    SimpleMesh m;

    // UV-координаты для каждой грани (0,0) - (1,1)
    // индексы граней как в твоем коде
    int faceVerts[6][4] = {
        {0,1,2,3}, // back
        {4,5,6,7}, // front  
        {0,4,7,3}, // left
        {1,5,6,2}, // right
        {3,2,6,7}, // top
        {0,1,5,4}  // bottom
    };

    // UV-координаты для каждой грани (одинаковые для всех граней)
    glm::vec2 faceUVs[4] = {
        glm::vec2(0.0f, 0.0f),  // нижний левый
        glm::vec2(1.0f, 0.0f),  // нижний правый  
        glm::vec2(1.0f, 1.0f),  // верхний правый
        glm::vec2(0.0f, 1.0f)   // верхний левый
    };

    for (int f = 0; f < 6; f++) {
        int base = m.verts.size();

        // добавляем 4 вершины грани
        m.verts.push_back(v[faceVerts[f][0]]);
        m.verts.push_back(v[faceVerts[f][1]]);
        m.verts.push_back(v[faceVerts[f][2]]);
        m.verts.push_back(v[faceVerts[f][3]]);

        // добавляем UV-координаты
        m.uvs.push_back(faceUVs[0]);
        m.uvs.push_back(faceUVs[1]);
        m.uvs.push_back(faceUVs[2]);
        m.uvs.push_back(faceUVs[3]);

        // цвета (если нужно в шейдере) - можно сделать серым или белым
        for (int i = 0; i < 4; i++)
            m.cols.push_back(glm::vec3(1.0f));  // белый цвет (текстура перекроет)

        // индексы треугольников
        m.inds.push_back(base + 0);
        m.inds.push_back(base + 1);
        m.inds.push_back(base + 2);

        m.inds.push_back(base + 0);
        m.inds.push_back(base + 2);
        m.inds.push_back(base + 3);
    }

    return m;
}

// функция генерации кабеля: L-образная лента, состоящая из сегментов (ribbon)
SimpleMesh make_cable(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, int segments, glm::vec3 color) {
    // кривой через три контрольные точки (квадратичная безье)
    SimpleMesh m;
    for (int i = 0; i <= segments; i++) {
        float t = (float)i / segments;
        // квадр. безье
        glm::vec3 p = (1 - t) * (1 - t) * p0 + 2 * (1 - t) * t * p1 + t * t * p2;
        // направление касательной
        glm::vec3 tangent = glm::normalize(2 * (1 - t) * (p1 - p0) + 2 * t * (p2 - p1));
        // нормаль в плоскости XZ
        glm::vec3 up = glm::vec3(0, 1, 0);
        glm::vec3 bit = glm::normalize(glm::cross(tangent, up));
        float width = 0.015f;
        glm::vec3 a = p - bit * width;
        glm::vec3 b = p + bit * width;
        m.verts.push_back(a);
        m.verts.push_back(b);
        m.cols.push_back(color);
        m.cols.push_back(color);
        if (i > 0) {
            GLuint base = (GLuint)m.verts.size() - 4;
            // два треугольника
            m.inds.push_back(base + 0);
            m.inds.push_back(base + 1);
            m.inds.push_back(base + 2);

            m.inds.push_back(base + 1);
            m.inds.push_back(base + 3);
            m.inds.push_back(base + 2);
        }
    }
    return m;
}

SimpleMesh make_colored_room() {
    SimpleMesh m;

    glm::vec3 center(0.0f);
    glm::vec3 size(6.0f, 4.0f, 6.0f);
    glm::vec3 hs = size * 0.5f;

    glm::vec3 v[8] = {
        center + glm::vec3(-hs.x, -hs.y, -hs.z), // 0
        center + glm::vec3(hs.x, -hs.y, -hs.z),  // 1
        center + glm::vec3(hs.x,  hs.y, -hs.z),  // 2
        center + glm::vec3(-hs.x,  hs.y, -hs.z), // 3

        center + glm::vec3(-hs.x, -hs.y,  hs.z), // 4
        center + glm::vec3(hs.x, -hs.y,  hs.z),  // 5
        center + glm::vec3(hs.x,  hs.y,  hs.z),  // 6
        center + glm::vec3(-hs.x,  hs.y,  hs.z)  // 7
    };

    // правильные 4 угла каждой стены:
    int faceVerts[6][4] = {
        {0,1,2,3}, // back
        {4,5,6,7}, // front
        {0,4,7,3}, // left
        {1,5,6,2}, // right
        {3,2,6,7}, // top
        {0,1,5,4}  // bottom
    };

    // цвета граней
    glm::vec3 colors[6] = {
        glm::vec3(0.94f, 0.90f, 0.75f),  // back     beige
        glm::vec3(0.94f, 0.90f, 0.75f),  // front    beige
        glm::vec3(0.94f, 0.90f, 0.82f),  // left     beige
        glm::vec3(0.94f, 0.90f, 0.82f),  // right    beige
        glm::vec3(1.0f, 0.97f, 0.70f),   // top      soft yellow
        glm::vec3(0.45f, 0.30f, 0.18f)   // bottom   brown floor
    };

    for (int f = 0; f < 6; f++) {
        int base = m.verts.size();

        // добавляем 4 угла текущей грани
        m.verts.push_back(v[faceVerts[f][0]]);
        m.verts.push_back(v[faceVerts[f][1]]);
        m.verts.push_back(v[faceVerts[f][2]]);
        m.verts.push_back(v[faceVerts[f][3]]);

        // цвет
        for (int i = 0; i < 4; i++)
            m.cols.push_back(colors[f]);

        // два треугольника
        m.inds.push_back(base + 0);
        m.inds.push_back(base + 1);
        m.inds.push_back(base + 2);

        m.inds.push_back(base + 0);
        m.inds.push_back(base + 2);
        m.inds.push_back(base + 3);
    }

    return m;
}



int main()
{
    GLFWwindow* window = InitAll(1024, 768, false);
    if (!window) { EndAll(); return -1; }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.85f, 0.9f, 0.95f, 1.0f);


    // В начале main() после инициализации OpenGL:
    GLuint phone_texture_id;

    // Загрузка текстуры
    glGenTextures(1, &phone_texture_id);
    glBindTexture(GL_TEXTURE_2D, phone_texture_id);

    // Загрузка изображения (нужна библиотека SOIL или stb_image)
    int width, height, channels;
    unsigned char* image_data = stbi_load("phone.png", &width, &height, &channels, 3);
    if (image_data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(image_data);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }


    // создаём модели
    Model room(window);
    Model phone(window);
    Model plug(window);
    Model cable(window);
    Model table(window);
    glm::vec3 table_pos(0.0f, -0.6f, 0.0f);  // начальная позиция стола
    
    // Убери старые вычисления px, pz из глобальной области и сделай так:
    float table_hx = 2.0f * 0.5f;   // половина ширины столешницы (по X)
    float table_hz = 1.0f * 0.5f;   // половина глубины столешницы (по Z)
    float table_y_bottom = -0.6f - 0.1f; // Y нижней грани столешницы (Y стола - половина толщины)

    float leg_width = 0.08f;
    float leg_height = 0.7f;
    float leg_half = leg_width * 0.5f;
    float inset_margin = 0.02f; // отступ от края

    // Правильные смещения ножек от центра стола:
    float px = table_hx - leg_half - inset_margin; // от центра до края стола минус половина ножки минус отступ
    float pz = table_hz - leg_half - inset_margin;

    // Y-координата центра ножки (такая же для всех):
    float leg_center_y = table_y_bottom - leg_height * 0.5f; // нижняя грань стола - половина высоты ножки

    auto make_leg = [&](float x, float z) {
        return make_box(
            glm::vec3(x, leg_center_y, z),     // позиция ножки
            glm::vec3(leg_width, leg_height, leg_width),
            glm::vec3(0.35f, 0.18f, 0.08f)
        );
        };

    // Создаем ножки с начальной позицией стола (0, -0.6, 0):
    SimpleMesh L1 = make_leg(0.0f + px, 0.0f + pz);  // правая передняя
    SimpleMesh L2 = make_leg(0.0f + px, 0.0f - pz);  // правая задняя
    SimpleMesh L3 = make_leg(0.0f - px, 0.0f + pz);  // левая передняя
    SimpleMesh L4 = make_leg(0.0f - px, 0.0f - pz);  // левая задняя

    Model leg1(window);
    leg1.load_coords(L1.verts.data(), L1.verts.size());
    leg1.load_colors(L1.cols.data(), L1.cols.size());
    leg1.load_indices(L1.inds.data(), L1.inds.size());
    Model leg2(window);
    leg2.load_coords(L2.verts.data(), L2.verts.size());
    leg2.load_colors(L2.cols.data(), L2.cols.size());
    leg2.load_indices(L2.inds.data(), L2.inds.size());
    Model leg3(window);
    leg3.load_coords(L3.verts.data(), L3.verts.size());
    leg3.load_colors(L3.cols.data(), L3.cols.size());
    leg3.load_indices(L3.inds.data(), L3.inds.size());
    Model leg4(window);
    leg4.load_coords(L4.verts.data(), L4.verts.size());
    leg4.load_colors(L4.cols.data(), L4.cols.size());
    leg4.load_indices(L4.inds.data(), L4.inds.size());

    // шейдеры для всех — один и тот же (fs использует u_time)
    room.load_shaders("vs.glsl", "fs.glsl");
    table.load_shaders("vs.glsl", "fs.glsl");
    phone.load_shaders("vs_phone.glsl", "fs_phone.glsl");
    plug.load_shaders("vs.glsl", "fs.glsl");
    cable.load_shaders("vs.glsl", "fsCable.glsl");

    // комната - большая инвертированная коробка
    SimpleMesh roomMesh = make_colored_room();
    room.load_coords(roomMesh.verts.data(), roomMesh.verts.size());
    room.load_colors(roomMesh.cols.data(), roomMesh.cols.size());
    room.load_indices(roomMesh.inds.data(), roomMesh.inds.size());

    // стол — коробка
    SimpleMesh tableMesh = make_box(table_pos, glm::vec3(2.0f, 0.2f, 1.0f), glm::vec3(0.6f, 0.3f, 0.1f));
    table.load_coords(tableMesh.verts.data(), tableMesh.verts.size());
    table.load_colors(tableMesh.cols.data(), tableMesh.cols.size());
    table.load_indices(tableMesh.inds.data(), tableMesh.inds.size());

    // телефон — тонкая коробка на столе (смещаем немного)
    glm::vec3 phone_pos = glm::vec3(0.3f, -1.09f, 0.0f);

    glm::vec3 phone_local_offset = glm::vec3(-0.3f, -1.09f, 0.0f);

    SimpleMesh phoneMesh = make_textured_box(phone_pos, glm::vec3(0.2f, 0.02f, 0.12f));
    phone.load_coords(phoneMesh.verts.data(), phoneMesh.verts.size());
    phone.load_colors(phoneMesh.cols.data(), phoneMesh.cols.size());
    phone.load_uvs(phoneMesh.uvs.data(), phoneMesh.uvs.size());  // добавь эту строку!
    phone.load_indices(phoneMesh.inds.data(), phoneMesh.inds.size());

    float phone_half_x = 0.1f;   // половина ширины телефона
    float phone_half_z = 0.06f;  // половина глубины телефона
    float max_table_x = table_hx - phone_local_offset.x - phone_half_x;   // когда правый край телефона упирается в правый край стола
    float min_table_x = -table_hx - phone_local_offset.x + phone_half_x;  // когда левый край телефона упирается в левый край стола
    float max_table_z = table_hz - phone_local_offset.z - phone_half_z;
    float min_table_z = -table_hz - phone_local_offset.z + phone_half_z;

    // вилка в стене (маленький бокс на стене)
    SimpleMesh plugMesh = make_box(glm::vec3(2.98f, -0.2f, 0.0f), glm::vec3(0.08f, 0.06f, 0.04f), glm::vec3(0.15f, 0.15f, 0.15f)); 
    plug.load_coords(plugMesh.verts.data(), plugMesh.verts.size());
    plug.load_colors(plugMesh.cols.data(), plugMesh.cols.size());
    plug.load_indices(plugMesh.inds.data(), plugMesh.inds.size());

    // кабель: от вилки к телефону; L-образная кривая (p0->p1->p2)
    glm::vec3 p0 = glm::vec3(2.96f, -0.2f, 0.0f);   // точка подключения к розетке (чуть внутрь)
    glm::vec3 p1 = glm::vec3(1.8f, -1.0f, 0.0f);    // промежуточная точка для дуги
    glm::vec3 p2 = glm::vec3(0.38f, -1.1f, 0.0f);   // точка подключения к нижней части телефона
    SimpleMesh cableMesh = make_cable(p0, p1, p2, 48, glm::vec3(0.1f, 0.1f, 0.1f));
    cable.load_coords(cableMesh.verts.data(), cableMesh.verts.size());
    cable.load_colors(cableMesh.cols.data(), cableMesh.cols.size());
    cable.load_indices(cableMesh.inds.data(), cableMesh.inds.size());

    // камера / управление
    glm::vec3 camPos = glm::vec3(-2.0f, 0.0f, 3.0f);
    float camYaw = 40.0f;
    float camPitch = -10.0f;

    float lastTime = (float)glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        float now = (float)glfwGetTime();
        float dt = now - lastTime;
        lastTime = now;

        // управление (WASD Q/E, стрелки для вращения)
        float speed = 2.0f;
        glm::vec3 forward(
            sin(glm::radians(camYaw)),
            0,
            -cos(glm::radians(camYaw))
        );
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camPos += forward * speed * dt;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camPos -= forward * speed * dt;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camPos -= right * speed * dt;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camPos += right * speed * dt;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camPos.y += speed * dt;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camPos.y -= speed * dt;

        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) table_pos.z += speed * dt;  // вперед (по Z)
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) table_pos.z -= speed * dt;  // назад
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) table_pos.x -= speed * dt;  // влево (по X)
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) table_pos.x += speed * dt;  // вправо


        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) camYaw += 60.0f * dt;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) camYaw -= 60.0f * dt;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) camPitch += 40.0f * dt;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) camPitch -= 40.0f * dt;

        // ограничение питча
        if (camPitch > 89.0f) camPitch = 89.0f;
        if (camPitch < -89.0f) camPitch = -89.0f;

        // матрицы
        glm::mat4 view = glm::lookAt(camPos,
            camPos + glm::vec3(sin(glm::radians(camYaw)) * cos(glm::radians(camPitch)),
                sin(glm::radians(camPitch)),
                -cos(glm::radians(camYaw)) * cos(glm::radians(camPitch))),
            glm::vec3(0, 1, 0));
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)WinWidth / (float)WinHeight, 0.1f, 100.0f);

        glViewport(0, 0, WinWidth, WinHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render function helper
        auto renderModel = [&](Model& m, glm::mat4 modelMat) {
            glm::mat4 MVP = projection * view * modelMat;
            GLuint prog = m.get_shader_programme();
            glUseProgram(prog);
            GLint id = glGetUniformLocation(prog, "MVP");
            if (id >= 0) glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(MVP));
            GLint id2 = glGetUniformLocation(prog, "ModelMat");
            if (id2 >= 0) glUniformMatrix4fv(id2, 1, GL_FALSE, glm::value_ptr(modelMat));
            GLint tid = glGetUniformLocation(prog, "u_time");
            if (tid >= 0) glUniform1f(tid, now);


            // Если это телефон - привяжи текстуру:
            if (&m == &phone) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, phone_texture_id);
                GLint texLoc = glGetUniformLocation(prog, "tex");
                if (texLoc >= 0) glUniform1i(texLoc, 0);
            }

            m.render(GL_TRIANGLES);
        };

        // Отрисовка: комната (не двигаем её - модельная матрица identity)
        renderModel(room, glm::translate(glm::mat4(1.0f), glm::vec3(0)));
        renderModel(phone, glm::mat4(1.0f));
        renderModel(plug, glm::mat4(1.0f));
        renderModel(cable, glm::mat4(1.0f));

        if (table_pos.x > max_table_x) table_pos.x = max_table_x;
        if (table_pos.x < min_table_x) table_pos.x = min_table_x;
        if (table_pos.z > max_table_z) table_pos.z = max_table_z;
        if (table_pos.z < min_table_z) table_pos.z = min_table_z;

        renderModel(table, glm::translate(glm::mat4(1.0f), table_pos));

        // Для ножек используем ту же логику - смещение относительно позиции стола:
        float px = table_hx - leg_half - inset_margin;
        float pz = table_hz - leg_half - inset_margin;
        float leg_center_y = table_pos.y - 0.1f - leg_height * 0.5f;

        renderModel(leg1, glm::translate(glm::mat4(1.0f), glm::vec3(table_pos.x + px, leg_center_y, table_pos.z + pz)));
        renderModel(leg2, glm::translate(glm::mat4(1.0f), glm::vec3(table_pos.x + px, leg_center_y, table_pos.z - pz)));
        renderModel(leg3, glm::translate(glm::mat4(1.0f), glm::vec3(table_pos.x - px, leg_center_y, table_pos.z + pz)));
        renderModel(leg4, glm::translate(glm::mat4(1.0f), glm::vec3(table_pos.x - px, leg_center_y, table_pos.z - pz)));

        glfwPollEvents();
        glfwSwapBuffers(window);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, 1);
    }

    EndAll();
    return 0;
}

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

int WinWidth;
int WinHeight;

struct SimpleMesh {
    std::vector<glm::vec3> verts;
    std::vector<glm::vec3> cols;
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
        float width = 0.03f;
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
        center + glm::vec3(-hs.x, -hs.y, -hs.z),
        center + glm::vec3(hs.x, -hs.y, -hs.z),
        center + glm::vec3(hs.x,  hs.y, -hs.z),
        center + glm::vec3(-hs.x,  hs.y, -hs.z),

        center + glm::vec3(-hs.x, -hs.y,  hs.z),
        center + glm::vec3(hs.x, -hs.y,  hs.z),
        center + glm::vec3(hs.x,  hs.y,  hs.z),
        center + glm::vec3(-hs.x,  hs.y,  hs.z)
    };

    GLuint idxs[] = {
        0,1,2, 0,2,3,  // back
        4,5,6, 4,6,7,  // front
        0,4,7, 0,7,3,  // left
        1,5,6, 1,6,2,  // right
        3,2,6, 3,6,7,  // top
        0,1,5, 0,5,4   // bottom
    };

    glm::vec3 colors[6] = {
        glm::vec3(0.8f, 0.8f, 1.0f),
        glm::vec3(1.0f, 0.8f, 0.8f),
        glm::vec3(0.8f, 1.0f, 0.8f),
        glm::vec3(1.0f, 1.0f, 0.7f),
        glm::vec3(0.9f, 0.9f, 0.9f),
        glm::vec3(0.5f, 0.4f, 0.3f)
    };

    // Выделяем 24 вершины (для 6 граней × 4 вершины)
    for (int f = 0; f < 6; f++) {
        int base = f * 4;

        // копируем вершины
        m.verts.push_back(v[idxs[f * 6 + 0]]);
        m.verts.push_back(v[idxs[f * 6 + 1]]);
        m.verts.push_back(v[idxs[f * 6 + 2]]);
        m.verts.push_back(v[idxs[f * 6 + 3]]);

        // цвета этой грани
        for (int i = 0; i < 4; i++)
            m.cols.push_back(colors[f]);

        // индексы (локально)
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

    // создаём модели
    Model room(window);
    Model table(window);
    Model phone(window);
    Model plug(window);
    Model cable(window);

    // шейдеры для всех — один и тот же (fs использует u_time)
    room.load_shaders("vs.glsl", "fs.glsl");
    table.load_shaders("vs.glsl", "fs.glsl");
    phone.load_shaders("vs.glsl", "fs.glsl");
    plug.load_shaders("vs.glsl", "fs.glsl");
    cable.load_shaders("vs.glsl", "fsCable.glsl");

    // комната - большая инвертированная коробка
    SimpleMesh roomMesh = make_colored_room();
    room.load_coords(roomMesh.verts.data(), roomMesh.verts.size());
    room.load_colors(roomMesh.cols.data(), roomMesh.cols.size());
    room.load_indices(roomMesh.inds.data(), roomMesh.inds.size());

    // стол — коробка
    SimpleMesh tableMesh = make_box(glm::vec3(0.0f, -1.2f, 0.0f), glm::vec3(2.0f, 0.2f, 1.0f), glm::vec3(0.6f, 0.3f, 0.1f));
    table.load_coords(tableMesh.verts.data(), tableMesh.verts.size());
    table.load_colors(tableMesh.cols.data(), tableMesh.cols.size());
    table.load_indices(tableMesh.inds.data(), tableMesh.inds.size());

    // телефон — тонкая коробка на столе (смещаем немного)
    SimpleMesh phoneMesh = make_box(glm::vec3(0.3f, -1.05f, 0.0f), glm::vec3(0.2f, 0.02f, 0.12f), glm::vec3(0.05f, 0.05f, 0.05f));
    phone.load_coords(phoneMesh.verts.data(), phoneMesh.verts.size());
    phone.load_colors(phoneMesh.cols.data(), phoneMesh.cols.size());
    phone.load_indices(phoneMesh.inds.data(), phoneMesh.inds.size());

    // вилка в стене (маленький бокс на стене)
    SimpleMesh plugMesh = make_box(glm::vec3(2.98f, -0.2f, 0.8f), glm::vec3(0.08f, 0.06f, 0.04f), glm::vec3(0.15f, 0.15f, 0.15f));
    plug.load_coords(plugMesh.verts.data(), plugMesh.verts.size());
    plug.load_colors(plugMesh.cols.data(), plugMesh.cols.size());
    plug.load_indices(plugMesh.inds.data(), plugMesh.inds.size());

    // кабель: от вилки к телефону; L-образная кривая (p0->p1->p2)
    glm::vec3 p0 = glm::vec3(2.92f, -0.25f, 0.82f);
    glm::vec3 p1 = glm::vec3(1.2f, -0.6f, 0.9f);
    glm::vec3 p2 = glm::vec3(0.38f, -1.05f, 0.0f);
    SimpleMesh cableMesh = make_cable(p0, p1, p2, 48, glm::vec3(0.1f, 0.1f, 0.1f));
    cable.load_coords(cableMesh.verts.data(), cableMesh.verts.size());
    cable.load_colors(cableMesh.cols.data(), cableMesh.cols.size());
    cable.load_indices(cableMesh.inds.data(), cableMesh.inds.size());

    // камера / управление
    glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 4.0f);
    float camYaw = 180.0f; // градусы, смотрим в -z
    float camPitch = 0.0f;

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
            m.render(GL_TRIANGLES);
            };

        // Отрисовка: комната (не двигаем её - модельная матрица identity)
        renderModel(room, glm::translate(glm::mat4(1.0f), glm::vec3(0)));
        renderModel(table, glm::mat4(1.0f));
        renderModel(phone, glm::mat4(1.0f));
        renderModel(plug, glm::mat4(1.0f));
        renderModel(cable, glm::mat4(1.0f));

        glfwPollEvents();
        glfwSwapBuffers(window);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, 1);
    }

    EndAll();
    return 0;
}

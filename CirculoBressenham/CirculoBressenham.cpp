#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

// Dimensiones de la ventana y de la grilla
const int ANCHO_VENTANA = 800;
const int ALTO_VENTANA = 800;

const int COLUMNAS = 40;
const int FILAS = 40;

// Matriz para guardar el estado de los píxeles/celdas pintadas
bool grilla[FILAS][COLUMNAS] = { false };

// Datos del círculo
int centroX = 20;
int centroY = 20;
int radio = 12;

// Estructura para registrar los pasos en consola
struct PasoBresenham {
    int k;
    int Pk;
    int x;
    int y;
};

std::vector<PasoBresenham> historialPasos;

// Función para pintar una celda en la grilla y aplicar simetría de 8 vías
void pintar8Puntos(int xc, int yc, int x, int y) {
    int puntos[8][2] = {
        { xc + x, yc + y },
        { xc - x, yc + y },
        { xc + x, yc - y },
        { xc - x, yc - y },
        { xc + y, yc + x },
        { xc - y, yc + x },
        { xc + y, yc - x },
        { xc - y, yc - x }
    };

    for (int i = 0; i < 8; i++) {
        int px = puntos[i][0];
        int py = puntos[i][1];

        // Verificar límites de la grilla
        if (px >= 0 && px < COLUMNAS && py >= 0 && py < FILAS) {
            grilla[py][px] = true;
        }
    }
}

// Algoritmo del punto medio de Bresenham para círculos según las diapositivas
void calcularCirculoBresenham(int xc, int yc, int r) {
    // Paso 1: Coordenadas iniciales (0, R)
    int x = 0;
    int y = r;

    // Paso 2: Parámetro de decisión inicial P0 = 1 - R
    int p = 1 - r;

    historialPasos.clear();
    int k = 0;

    // Paso 3 al 5: Bucle mientras x <= y (cobertura del primer octante)
    while (x <= y) {
        // Graficar los 8 puntos simétricos en la grilla
        pintar8Puntos(xc, yc, x, y);

        int P_actual = p;

        // Evaluar el parámetro de decisión
        if (p < 0) {
            // Caso 1: Pk < 0 -> X_{k+1} = Xk + 1, Y_{k+1} = Yk
            p = p + 2 * x + 3; // o p + 2 * (x + 1) + 1
        }
        else {
            // Caso 2: Pk >= 0 -> X_{k+1} = Xk + 1, Y_{k+1} = Yk - 1
            y--;
            p = p + 2 * (x - y) + 1;
        }

        x++;

        historialPasos.push_back({ k, P_actual, x - 1, (p < 0) ? y : y + 1 });

        std::cout << std::setw(5) << k
            << std::setw(10) << P_actual
            << std::setw(8) << "(" << (x - 1) << ", " << ((p < 0) ? y : y + 1) << ")"
            << std::setw(12) << p << "\n";
        k++;
    }
}

// Dibujar una celda (píxel) llena
void dibujarCelda(int col, int fila, float r, float g, float b) {
    float anchoCelda = 2.0f / COLUMNAS;
    float altoCelda = 2.0f / FILAS;

    float x0 = -1.0f + col * anchoCelda;
    float y0 = -1.0f + fila * altoCelda;
    float x1 = x0 + anchoCelda;
    float y1 = y0 + altoCelda;

    glColor3f(r, g, b);
    glBegin(GL_TRIANGLES);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);

    glVertex2f(x0, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();
}

// Dibujar la rejilla
void dibujarGrilla() {
    glColor3f(0.25f, 0.25f, 0.25f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);

    // Líneas verticales
    for (int col = 0; col <= COLUMNAS; col++) {
        float x = -1.0f + col * (2.0f / COLUMNAS);
        glVertex2f(x, -1.0f);
        glVertex2f(x, 1.0f);
    }

    // Líneas horizontales
    for (int fila = 0; fila <= FILAS; fila++) {
        float y = -1.0f + fila * (2.0f / FILAS);
        glVertex2f(-1.0f, y);
        glVertex2f(1.0f, y);
    }
    glEnd();
}

// Dibujar el círculo teórico continuo en color rojo
void dibujarCirculoIdeal(int xc, int yc, int r) {
    float anchoCelda = 2.0f / COLUMNAS;
    float altoCelda = 2.0f / FILAS;

    float centroX_NDC = -1.0f + (xc + 0.5f) * anchoCelda;
    float centroY_NDC = -1.0f + (yc + 0.5f) * altoCelda;
    float radioX_NDC = r * anchoCelda;
    float radioY_NDC = r * altoCelda;

    glColor3f(1.0f, 0.2f, 0.2f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 100; i++) {
        float angulo = 2.0f * 3.1415926f * i / 100.0f;
        float x = centroX_NDC + radioX_NDC * cosf(angulo);
        float y = centroY_NDC + radioY_NDC * sinf(angulo);
        glVertex2f(x, y);
    }
    glEnd();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    // 1. Inicializar GLFW
    if (!glfwInit()) {
        std::cerr << "Error al inicializar GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    GLFWwindow* window = glfwCreateWindow(ANCHO_VENTANA, ALTO_VENTANA, "Bresenham Circle - OpenGL (GLAD/GLFW)", NULL, NULL);
    if (!window) {
        std::cerr << "Error al crear la ventana de GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 2. Cargar punteros de funciones con GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Error al inicializar GLAD" << std::endl;
        return -1;
    }

    // 3. Ejecutar el algoritmo de Bresenham para el círculo
    calcularCirculoBresenham(centroX, centroY, radio);

    // 4. Bucle principal de renderizado
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Renderizar píxeles rasterizados en la grilla (Verde)
        for (int fila = 0; fila < FILAS; fila++) {
            for (int col = 0; col < COLUMNAS; col++) {
                if (grilla[fila][col]) {
                    dibujarCelda(col, fila, 0.2f, 0.8f, 0.4f);
                }
            }
        }

        // Resaltar el centro en amarillo
        dibujarCelda(centroX, centroY, 1.0f, 0.9f, 0.2f);

        // Dibujar grilla
        dibujarGrilla();

        // Dibujar trayectoria matemática ideal en rojo
        dibujarCirculoIdeal(centroX, centroY, radio);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
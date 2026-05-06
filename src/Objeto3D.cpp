/* Objeto 3D - Atividade Acadêmica Computação Gráfica
 *
 * Controles:
 *   W/S       - translação em Y (cima/baixo)
 *   A/D       - translação em X (esquerda/direita)
 *   +/-       - escala uniforme (aumentar/diminuir)
 *   X/Y/Z     - rotação incremental no respectivo eixo (acumula, segure para girar)
 *   TAB       - alterna objeto selecionado: 0 -> 1 -> 2 -> TODOS -> 0 -> ...
 *   ESC       - fecha a janela
 */

#include <iostream>
#include <string>
#include <vector>

using namespace std;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void key_callback(GLFWwindow* janela, int tecla, int scancode, int acao, int modo);
int inicializarShader();
int inicializarGeometria();

const GLuint LARGURA = 1000, ALTURA = 1000;

const GLchar* fonteVertice =
    "#version 450\n"
    "layout (location = 0) in vec3 posicao;\n"
    "layout (location = 1) in vec3 cor;\n"
    "uniform mat4 model;\n"
    "out vec4 corFinal;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = model * vec4(posicao, 1.0);\n"
    "    corFinal = vec4(cor, 1.0);\n"
    "}\0";

const GLchar* fonteFragmento =
    "#version 450\n"
    "in vec4 corFinal;\n"
    "out vec4 cor;\n"
    "void main()\n"
    "{\n"
    "    cor = corFinal;\n"
    "}\n\0";

struct Objeto {
    glm::vec3 posicao;
    float escala;
    float anguloX, anguloY, anguloZ;

    Objeto(glm::vec3 pos, float s = 0.25f)
        : posicao(pos), escala(s),
          anguloX(0.0f), anguloY(0.0f), anguloZ(0.0f) {}
};

vector<Objeto> objetos;

// 0, 1, 2 = objeto selecionado
int modoAtivo = 0;

const float PASSO_TRANSLACAO = 0.08f;
const float PASSO_ESCALA     = 0.04f;
const float ESCALA_MIN       = 0.04f;
const float PASSO_ROTACAO    = glm::radians(7.0f);

// Aplica uma função ao objeto selecionado
template<typename Fn>
void aplicarAoSelecionado(Fn fn) {
    fn(objetos[modoAtivo]);
}

int main()
{
    glfwInit();

    GLFWwindow* janela = glfwCreateWindow(LARGURA, ALTURA, "Objeto 3D", nullptr, nullptr);
    glfwMakeContextCurrent(janela);
    glfwSetKeyCallback(janela, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Falha ao inicializar GLAD" << endl;
        return -1;
    }

    const GLubyte* placa    = glGetString(GL_RENDERER);
    const GLubyte* versaoGL = glGetString(GL_VERSION);
    cout << "Renderer: " << placa << endl;
    cout << "OpenGL version: " << versaoGL << endl;

    int largura, altura;
    glfwGetFramebufferSize(janela, &largura, &altura);
    glViewport(0, 0, largura, altura);

    GLuint programaShader = inicializarShader();
    GLuint vao            = inicializarGeometria();

    glUseProgram(programaShader);
    GLint locModelo = glGetUniformLocation(programaShader, "model");

    glEnable(GL_DEPTH_TEST);

    // Três objetos dispostos em triângulo
    objetos.push_back(Objeto(glm::vec3(-0.45f, -0.3f, 0.0f)));
    objetos.push_back(Objeto(glm::vec3( 0.45f, -0.3f, 0.0f)));
    objetos.push_back(Objeto(glm::vec3( 0.00f,  0.5f, 0.0f)));

    while (!glfwWindowShouldClose(janela))
    {
        glfwPollEvents();

        string tituloJanela = "Cubo 3D | Júlia Oliveira";
        glfwSetWindowTitle(janela, tituloJanela.c_str());

        glClearColor(0.08f, 0.12f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(vao);
        for (int i = 0; i < (int)objetos.size(); i++)
        {
            glm::mat4 matriz = glm::mat4(1.0f);

            // Ordem TRS: primeiro escala, depois rotação, depois translação
            matriz = glm::translate(matriz, objetos[i].posicao);

            // Rotação acumulada nos três eixos independentemente
            matriz = glm::rotate(matriz, objetos[i].anguloX, glm::vec3(1.0f, 0.0f, 0.0f));
            matriz = glm::rotate(matriz, objetos[i].anguloY, glm::vec3(0.0f, 1.0f, 0.0f));
            matriz = glm::rotate(matriz, objetos[i].anguloZ, glm::vec3(0.0f, 0.0f, 1.0f));

            matriz = glm::scale(matriz, glm::vec3(objetos[i].escala));

            glUniformMatrix4fv(locModelo, 1, GL_FALSE, glm::value_ptr(matriz));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        glfwSwapBuffers(janela);
    }

    glDeleteVertexArrays(1, &vao);
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* janela, int tecla, int scancode, int acao, int modo)
{
    if (tecla == GLFW_KEY_ESCAPE && acao == GLFW_PRESS)
        glfwSetWindowShouldClose(janela, GL_TRUE);

    // Ciclo: Objeto 0 -> Objeto 1 -> Objeto 2 -> Objeto 0 -> ...
    if (tecla == GLFW_KEY_TAB && acao == GLFW_PRESS)
        modoAtivo = (modoAtivo + 1) % (int)objetos.size();

    // Translação e escala respondem a PRESS e REPEAT (segurar a tecla)
    if (acao == GLFW_PRESS || acao == GLFW_REPEAT)
    {
        if (tecla == GLFW_KEY_A)
            aplicarAoSelecionado([](Objeto& o){ o.posicao.x -= PASSO_TRANSLACAO; });
        if (tecla == GLFW_KEY_D)
            aplicarAoSelecionado([](Objeto& o){ o.posicao.x += PASSO_TRANSLACAO; });
        if (tecla == GLFW_KEY_W)
            aplicarAoSelecionado([](Objeto& o){ o.posicao.y += PASSO_TRANSLACAO; });
        if (tecla == GLFW_KEY_S)
            aplicarAoSelecionado([](Objeto& o){ o.posicao.y -= PASSO_TRANSLACAO; });

        if (tecla == GLFW_KEY_EQUAL)
            aplicarAoSelecionado([](Objeto& o){ o.escala += PASSO_ESCALA; });
        if (tecla == GLFW_KEY_MINUS)
            aplicarAoSelecionado([](Objeto& o){
                o.escala -= PASSO_ESCALA;
                if (o.escala < ESCALA_MIN) o.escala = ESCALA_MIN;
            });
    }

    // Rotação incremental: acumula ângulo, responde a PRESS e REPEAT
    if (acao == GLFW_PRESS || acao == GLFW_REPEAT)
    {
        if (tecla == GLFW_KEY_X)
            aplicarAoSelecionado([](Objeto& o){ o.anguloX += PASSO_ROTACAO; });
        if (tecla == GLFW_KEY_Y)
            aplicarAoSelecionado([](Objeto& o){ o.anguloY += PASSO_ROTACAO; });
        if (tecla == GLFW_KEY_Z)
            aplicarAoSelecionado([](Objeto& o){ o.anguloZ += PASSO_ROTACAO; });
    }
}

int inicializarShader()
{
    GLuint shaderVertice = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shaderVertice, 1, &fonteVertice, NULL);
    glCompileShader(shaderVertice);
    GLint compilado;
    GLchar mensagemErro[512];
    glGetShaderiv(shaderVertice, GL_COMPILE_STATUS, &compilado);
    if (!compilado) {
        glGetShaderInfoLog(shaderVertice, 512, NULL, mensagemErro);
        cout << "ERRO::VERTEX_SHADER: " << mensagemErro << endl;
    }

    GLuint shaderFragmento = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shaderFragmento, 1, &fonteFragmento, NULL);
    glCompileShader(shaderFragmento);
    glGetShaderiv(shaderFragmento, GL_COMPILE_STATUS, &compilado);
    if (!compilado) {
        glGetShaderInfoLog(shaderFragmento, 512, NULL, mensagemErro);
        cout << "ERRO::FRAGMENT_SHADER: " << mensagemErro << endl;
    }

    GLuint programa = glCreateProgram();
    glAttachShader(programa, shaderVertice);
    glAttachShader(programa, shaderFragmento);
    glLinkProgram(programa);
    glGetProgramiv(programa, GL_LINK_STATUS, &compilado);
    if (!compilado) {
        glGetProgramInfoLog(programa, 512, NULL, mensagemErro);
        cout << "ERRO::SHADER_PROGRAM: " << mensagemErro << endl;
    }

    glDeleteShader(shaderVertice);
    glDeleteShader(shaderFragmento);
    return programa;
}

int inicializarGeometria()
{
    // 6 faces × 2 triângulos × 3 vértices = 36 vértices
    // Formato por vértice: x, y, z, r, g, b
    GLfloat vertices[] = {

        // Face frontal (+Z) — Laranja
        -0.5f, -0.5f,  0.5f,  1.0f, 0.55f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.55f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.55f, 0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.55f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.55f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.55f, 0.0f,

        // Face traseira (-Z) — Roxo
         0.5f, -0.5f, -0.5f,  0.55f, 0.0f, 0.8f,
        -0.5f, -0.5f, -0.5f,  0.55f, 0.0f, 0.8f,
        -0.5f,  0.5f, -0.5f,  0.55f, 0.0f, 0.8f,
         0.5f, -0.5f, -0.5f,  0.55f, 0.0f, 0.8f,
        -0.5f,  0.5f, -0.5f,  0.55f, 0.0f, 0.8f,
         0.5f,  0.5f, -0.5f,  0.55f, 0.0f, 0.8f,

        // Face esquerda (-X) — Turquesa
        -0.5f, -0.5f, -0.5f,  0.0f, 0.75f, 0.65f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.75f, 0.65f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.75f, 0.65f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.75f, 0.65f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.75f, 0.65f,
        -0.5f,  0.5f, -0.5f,  0.0f, 0.75f, 0.65f,

        // Face direita (+X) — Rosa
         0.5f, -0.5f,  0.5f,  1.0f, 0.35f, 0.65f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.35f, 0.65f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.35f, 0.65f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.35f, 0.65f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.35f, 0.65f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.35f, 0.65f,

        // Face superior (+Y) — Branco
        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,

        // Face inferior (-Y) — Marrom
        -0.5f, -0.5f, -0.5f,  0.75f, 0.25f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.75f, 0.25f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.75f, 0.25f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.75f, 0.25f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.75f, 0.25f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.75f, 0.25f, 0.0f,
    };

    GLuint vbo, vao;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Atributo 0: posição (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Atributo 1: cor (r, g, b)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return vao;
}

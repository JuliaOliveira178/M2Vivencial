/* Objeto 3D - Atividade Acadêmica Computação Gráfica - Módulo 3
 * Júlia Oliveira
 *
 * Controles:
 *   TAB       - alterna objeto selecionado: 0 -> 1 -> 2 -> 0 -> ...
 *   R         - ativa modo Girar   -> X/Y/Z rotacionam no eixo respectivo
 *   T         - ativa modo Transladar -> W/A/D/Seta↑↓←→ transladam
 *   S         - ativa modo Escalar -> +/- escala uniforme
 *   ESC       - fecha a janela
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void key_callback(GLFWwindow* janela, int tecla, int scancode, int acao, int modo);
GLuint carregarOBJ(const string& caminho, int& nVertices, glm::vec3 cor);
int inicializarShader();

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
    GLuint vao;
    int nVertices;
    glm::vec3 posicao;
    float escala;
    float anguloX, anguloY, anguloZ;

    Objeto(GLuint v, int nv, glm::vec3 pos, float s = 0.25f)
        : vao(v), nVertices(nv), posicao(pos), escala(s),
          anguloX(0.0f), anguloY(0.0f), anguloZ(0.0f) {}
};

vector<Objeto> objetos;

// Índice do objeto atualmente selecionado
int modoAtivo = 0;

enum Modo { GIRAR, TRANSLADAR, ESCALAR };
Modo modoTransformacao = TRANSLADAR;

const float PASSO_TRANSLACAO = 0.08f;
const float PASSO_ESCALA     = 0.04f;
const float ESCALA_MIN       = 0.04f;
const float PASSO_ROTACAO    = glm::radians(7.0f);

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

    cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
    cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;

    int largura, altura;
    glfwGetFramebufferSize(janela, &largura, &altura);
    glViewport(0, 0, largura, altura);

    GLuint programaShader = inicializarShader();
    glUseProgram(programaShader);
    GLint locModelo = glGetUniformLocation(programaShader, "model");

    glEnable(GL_DEPTH_TEST);

    // Três Suzannes em triângulo com cores laranja, turquesa e roxo
    int nv;
    GLuint vao0 = carregarOBJ("assets/modelo.obj", nv, glm::vec3(1.0f, 0.55f, 0.0f));
    objetos.push_back(Objeto(vao0, nv, glm::vec3(-0.45f, -0.3f, 0.0f), 0.2f));

    GLuint vao1 = carregarOBJ("assets/modelo.obj", nv, glm::vec3(0.0f, 0.75f, 0.65f));
    objetos.push_back(Objeto(vao1, nv, glm::vec3( 0.45f, -0.3f, 0.0f), 0.2f));

    GLuint vao2 = carregarOBJ("assets/modelo.obj", nv, glm::vec3(0.55f, 0.0f, 0.8f));
    objetos.push_back(Objeto(vao2, nv, glm::vec3( 0.00f,  0.5f, 0.0f), 0.2f));

    if (vao0 == (GLuint)-1 || vao1 == (GLuint)-1 || vao2 == (GLuint)-1) {
        cerr << "Falha ao carregar modelos OBJ." << endl;
        glfwTerminate();
        return -1;
    }

    while (!glfwWindowShouldClose(janela))
    {
        glfwPollEvents();

        string nomeObjeto = "Objeto " + to_string(modoAtivo);
        string nomeModo = (modoTransformacao == GIRAR)      ? "GIRAR"
                        : (modoTransformacao == TRANSLADAR) ? "TRANSLADAR"
                        :                                     "ESCALAR";
        string dica = (modoTransformacao == GIRAR)      ? "  setas=girar XYZ=eixo"
                    : (modoTransformacao == TRANSLADAR)  ? "  setas=mover"
                    :                                      "  setas/+/-=escala";
        string tituloJanela = "Objeto 3D | Julia Oliveira  |  [" + nomeObjeto + "]"
                            + "  [" + nomeModo + "]"
                            + "  |  R/T/S=modo  TAB=selecionar" + dica;
        glfwSetWindowTitle(janela, tituloJanela.c_str());

        glClearColor(0.08f, 0.12f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (int i = 0; i < (int)objetos.size(); i++)
        {
            glm::mat4 matriz = glm::mat4(1.0f);
            matriz = glm::translate(matriz, objetos[i].posicao);
            matriz = glm::rotate(matriz, objetos[i].anguloX, glm::vec3(1.0f, 0.0f, 0.0f));
            matriz = glm::rotate(matriz, objetos[i].anguloY, glm::vec3(0.0f, 1.0f, 0.0f));
            matriz = glm::rotate(matriz, objetos[i].anguloZ, glm::vec3(0.0f, 0.0f, 1.0f));
            matriz = glm::scale(matriz, glm::vec3(objetos[i].escala));

            glUniformMatrix4fv(locModelo, 1, GL_FALSE, glm::value_ptr(matriz));
            glBindVertexArray(objetos[i].vao);
            glDrawArrays(GL_TRIANGLES, 0, objetos[i].nVertices);
        }
        glBindVertexArray(0);

        glfwSwapBuffers(janela);
    }

    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* janela, int tecla, int scancode, int acao, int modo)
{
    if (tecla == GLFW_KEY_ESCAPE && acao == GLFW_PRESS)
        glfwSetWindowShouldClose(janela, GL_TRUE);

    // Ciclo: Objeto 0 -> 1 -> 2 -> 0 -> ...
    if (tecla == GLFW_KEY_TAB && acao == GLFW_PRESS)
        modoAtivo = (modoAtivo + 1) % (int)objetos.size();

    // Alternar modo de transformação com R / T / S
    if (acao == GLFW_PRESS) {
        if (tecla == GLFW_KEY_R) modoTransformacao = GIRAR;
        if (tecla == GLFW_KEY_T) modoTransformacao = TRANSLADAR;
        if (tecla == GLFW_KEY_S) modoTransformacao = ESCALAR;
    }

    if (acao == GLFW_PRESS || acao == GLFW_REPEAT)
    {
        if (modoTransformacao == TRANSLADAR) {
            if (tecla == GLFW_KEY_LEFT)
                aplicarAoSelecionado([](Objeto& o){ o.posicao.x -= PASSO_TRANSLACAO; });
            if (tecla == GLFW_KEY_RIGHT)
                aplicarAoSelecionado([](Objeto& o){ o.posicao.x += PASSO_TRANSLACAO; });
            if (tecla == GLFW_KEY_UP)
                aplicarAoSelecionado([](Objeto& o){ o.posicao.y += PASSO_TRANSLACAO; });
            if (tecla == GLFW_KEY_DOWN)
                aplicarAoSelecionado([](Objeto& o){ o.posicao.y -= PASSO_TRANSLACAO; });
        }

        if (modoTransformacao == ESCALAR) {
            if (tecla == GLFW_KEY_EQUAL || tecla == GLFW_KEY_UP)
                aplicarAoSelecionado([](Objeto& o){ o.escala += PASSO_ESCALA; });
            if (tecla == GLFW_KEY_MINUS || tecla == GLFW_KEY_DOWN)
                aplicarAoSelecionado([](Objeto& o){
                    o.escala -= PASSO_ESCALA;
                    if (o.escala < ESCALA_MIN) o.escala = ESCALA_MIN;
                });
        }

        if (modoTransformacao == GIRAR) {
            if (tecla == GLFW_KEY_LEFT)
                aplicarAoSelecionado([](Objeto& o){ o.anguloY -= PASSO_ROTACAO; });
            if (tecla == GLFW_KEY_RIGHT)
                aplicarAoSelecionado([](Objeto& o){ o.anguloY += PASSO_ROTACAO; });
            if (tecla == GLFW_KEY_UP)
                aplicarAoSelecionado([](Objeto& o){ o.anguloX -= PASSO_ROTACAO; });
            if (tecla == GLFW_KEY_DOWN)
                aplicarAoSelecionado([](Objeto& o){ o.anguloX += PASSO_ROTACAO; });
            if (tecla == GLFW_KEY_X)
                aplicarAoSelecionado([](Objeto& o){ o.anguloX += PASSO_ROTACAO; });
            if (tecla == GLFW_KEY_Y)
                aplicarAoSelecionado([](Objeto& o){ o.anguloY += PASSO_ROTACAO; });
            if (tecla == GLFW_KEY_Z)
                aplicarAoSelecionado([](Objeto& o){ o.anguloZ += PASSO_ROTACAO; });
        }
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

GLuint carregarOBJ(const string& caminho, int& nVertices, glm::vec3 cor)
{
    vector<glm::vec3> vertices;
    vector<glm::vec2> coordTex;
    vector<glm::vec3> normais;
    vector<GLfloat> buffer;

    ifstream arq(caminho);
    if (!arq.is_open()) {
        cerr << "Erro ao abrir o arquivo " << caminho << endl;
        return (GLuint)-1;
    }

    string linha;
    while (getline(arq, linha)) {
        istringstream ss(linha);
        string palavra;
        ss >> palavra;

        if (palavra == "v") {
            glm::vec3 v;
            ss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        } else if (palavra == "vt") {
            glm::vec2 vt;
            ss >> vt.s >> vt.t;
            coordTex.push_back(vt);
        } else if (palavra == "vn") {
            glm::vec3 vn;
            ss >> vn.x >> vn.y >> vn.z;
            normais.push_back(vn);
        } else if (palavra == "f") {
            string token;
            while (ss >> token) {
                int vi = 0, ti = 0, ni = 0;
                istringstream ssf(token);
                string idx;
                if (getline(ssf, idx, '/')) vi = !idx.empty() ? stoi(idx) - 1 : 0;
                if (getline(ssf, idx, '/')) ti = !idx.empty() ? stoi(idx) - 1 : 0;
                if (getline(ssf, idx))      ni = !idx.empty() ? stoi(idx) - 1 : 0;

                buffer.push_back(vertices[vi].x);
                buffer.push_back(vertices[vi].y);
                buffer.push_back(vertices[vi].z);
                buffer.push_back(cor.r);
                buffer.push_back(cor.g);
                buffer.push_back(cor.b);
            }
        }
    }
    arq.close();

    GLuint vbo, vao;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(GLfloat), buffer.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    nVertices = (int)(buffer.size() / 6);
    return vao;
}

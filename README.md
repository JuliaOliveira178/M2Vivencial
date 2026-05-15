# M2 Vivencial — Objeto 3D
**Júlia Oliveira | Computação Gráfica**

---

Cena interativa com três objetos 3D carregados a partir de arquivo `.obj`. O usuário pode selecionar individualmente cada objeto e aplicar transformações via teclado.

---

## Compilação

Requisitos: **CMake 3.10+** e compilador **C++17**.

```bash
mkdir build
cd build
cmake ..
cmake --build .
.\Objeto3D.exe
```

> GLFW e GLM são baixados automaticamente pelo CMake na primeira compilação.

---

## Como usar

O título da janela sempre mostra qual objeto está selecionado e qual modo está ativo.

**Seleção**
- `TAB` — passa para o próximo objeto (0 → 1 → 2 → 0)

**Modos de transformação**
- `R` — Girar
- `T` — Transladar  
- `S` — Escalar

**Ações com as setas (conforme o modo ativo)**

| Modo | `←` `→` | `↑` `↓` |
|------|---------|---------|
| Girar | Eixo Y | Eixo X |
| Transladar | Eixo X | Eixo Y |
| Escalar | — | Aumentar / Diminuir |

No modo **Girar**, as teclas `X` `Y` `Z` rotacionam diretamente no eixo correspondente.  
No modo **Escalar**, `+` e `-` também funcionam.

`ESC` fecha a aplicação.

---

## Stack

| | |
|---|---|
| Renderização | OpenGL 4.5 / GLSL 450 |
| Janela e input | GLFW 3.4 |
| Matemática 3D | GLM |
| Loader OpenGL | GLAD |

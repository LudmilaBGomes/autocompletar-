#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ncurses.h>
#include <ctype.h> // Para isprint

#define MAX_WORD_LENGTH 100

typedef struct No {
    char palavra[MAX_WORD_LENGTH];
    struct No *esquerda;
    struct No *direita;
} No;

// Protótipos de funções
bool palavraExiste(No *raiz, const char *palavra);
void removerPalavraArquivo(const char *palavra);
No* removerPalavra(No *raiz, const char *palavra);
No* encontrarMinimo(No *raiz);
void reescreverArquivo(No *raiz, FILE *arquivo);

// Função para criar um novo nó da árvore
No* criarNo(const char *palavra) {
    No *novo = (No*) malloc(sizeof(No));
    strcpy(novo->palavra, palavra);
    novo->esquerda = NULL;
    novo->direita = NULL;
    return novo;
}

// Função para inserir uma palavra na árvore
void inserirPalavra(No **raiz, const char *palavra) {
    if (*raiz == NULL) {
        *raiz = criarNo(palavra);
        return;
    }

    int cmp = strcmp(palavra, (*raiz)->palavra);
    if (cmp < 0) {
        inserirPalavra(&((*raiz)->esquerda), palavra);
    } else if (cmp > 0) {
        inserirPalavra(&((*raiz)->direita), palavra);
    }
}

// Função para buscar todas as palavras que começam com o prefixo
void buscarPrefixo(No *raiz, const char *prefixo, No **resultados, int *count, int maxResultados) {
    if (raiz == NULL || *count >= maxResultados) return;

    int cmp = strncmp(raiz->palavra, prefixo, strlen(prefixo));
    if (cmp == 0) {
        resultados[*count] = raiz;
        (*count)++;
    }

    buscarPrefixo(raiz->esquerda, prefixo, resultados, count, maxResultados);
    buscarPrefixo(raiz->direita, prefixo, resultados, count, maxResultados);
}

// Função para liberar a memória da árvore
void liberarArvore(No *raiz) {
    if (raiz == NULL) return;

    liberarArvore(raiz->esquerda);
    liberarArvore(raiz->direita);
    free(raiz);
}

// Função para adicionar uma palavra ao arquivo dicionario.dat
void adicionarPalavraArquivo(const char *palavra) {
    FILE *arquivo = fopen("dicionario.dat", "r+");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo dicionario.dat para adição.\n");
        return;
    }

    // Posiciona o ponteiro de arquivo no final do arquivo
    fseek(arquivo, 0, SEEK_END);

    if (fprintf(arquivo, "%s\n", palavra) < 0) {
        printf("Erro ao escrever no arquivo dicionario.dat.\n");
        fclose(arquivo);
        return;
    }

    fclose(arquivo);
}

// Função para remover uma palavra do arquivo dicionario.dat
void removerPalavraArquivo(const char *palavra) {
    FILE *arquivoLeitura = fopen("dicionario.dat", "r");
    if (arquivoLeitura == NULL) {
        printf("Erro ao abrir o arquivo dicionario.dat para remoção.\n");
        return;
    }

    FILE *arquivoTemp = fopen("temp.dat", "w");
    if (arquivoTemp == NULL) {
        printf("Erro ao criar arquivo temporário.\n");
        fclose(arquivoLeitura);
        return;
    }

    char buffer[MAX_WORD_LENGTH];
    while (fscanf(arquivoLeitura, "%s", buffer) != EOF) {
        if (strcmp(buffer, palavra) != 0) {
            fprintf(arquivoTemp, "%s\n", buffer);
        }
    }

    fclose(arquivoLeitura);
    fclose(arquivoTemp);

    // Remover o arquivo original e renomear o temporário
    if (remove("dicionario.dat") != 0) {
        printf("Erro ao remover o arquivo dicionario.dat.\n");
        return;
    }
    if (rename("temp.dat", "dicionario.dat") != 0) {
        printf("Erro ao renomear o arquivo temporário.\n");
        return;
    }
}

// Função para verificar se a palavra existe na árvore
bool palavraExiste(No *raiz, const char *palavra) {
    if (raiz == NULL) return false;

    int cmp = strcmp(palavra, raiz->palavra);
    if (cmp < 0) {
        return palavraExiste(raiz->esquerda, palavra);
    } else if (cmp > 0) {
        return palavraExiste(raiz->direita, palavra);
    } else {
        return true; // Encontrou a palavra na árvore
    }
}

// Função para encontrar o nó com o menor valor (usado na remoção)
No* encontrarMinimo(No *raiz) {
    while (raiz->esquerda != NULL) raiz = raiz->esquerda;
    return raiz;
}

// Função para remover uma palavra da árvore
No* removerPalavra(No *raiz, const char *palavra) {
    if (raiz == NULL) return NULL;

    int cmp = strcmp(palavra, raiz->palavra);
    if (cmp < 0) {
        raiz->esquerda = removerPalavra(raiz->esquerda, palavra);
    } else if (cmp > 0) {
        raiz->direita = removerPalavra(raiz->direita, palavra);
    } else {
        if (raiz->esquerda == NULL) {
            No *temp = raiz->direita;
            free(raiz);
            return temp;
        } else if (raiz->direita == NULL) {
            No *temp = raiz->esquerda;
            free(raiz);
            return temp;
        }

        No *temp = encontrarMinimo(raiz->direita);
        strcpy(raiz->palavra, temp->palavra);
        raiz->direita = removerPalavra(raiz->direita, temp->palavra);
    }

    return raiz;
}

// Função para reescrever o arquivo de dicionário com a árvore atualizada
void reescreverArquivo(No *raiz, FILE *arquivo) {
    if (raiz == NULL) return;

    reescreverArquivo(raiz->esquerda, arquivo);
    fprintf(arquivo, "%s\n", raiz->palavra);
    reescreverArquivo(raiz->direita, arquivo);
}

// Função principal
int main() {
    No *raiz = NULL;
    FILE *arquivoLeitura = fopen("dicionario.dat", "r"); // Abre para leitura

    if (arquivoLeitura == NULL) {
        printf("Erro ao abrir o arquivo dicionario.dat\n");
        return 1;
    }

    char palavra[MAX_WORD_LENGTH];
    while (fscanf(arquivoLeitura, "%s", palavra) != EOF) {
        inserirPalavra(&raiz, palavra);
    }
    fclose(arquivoLeitura);

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    printw("Digite o prefixo para autocompletar (ou '#' para sair):\n");
    refresh();

    char prefixo[MAX_WORD_LENGTH] = {0};
    No *resultados[100]; // Supondo no máximo 100 resultados
    int count = 0;
    int currentIndex = 0;
    int ch;
    int cursorPos = 0;
    int linhaAtual = 1;
    bool modoExclusao = false;

    while (true) {
        ch = getch();

        if (ch == '\t') { // Tecla Tab
            if (strlen(prefixo) > 0) {
                if (count == 0) {
                    // Buscar todas as palavras que começam com o prefixo
                    buscarPrefixo(raiz, prefixo, resultados, &count, 100);
                }

                if (count > 0) {
                    strcpy(palavra, resultados[currentIndex]->palavra);
                    move(linhaAtual, 0);
                    clrtoeol();
                    printw("%s", palavra);
                    cursorPos = strlen(palavra);
                    move(linhaAtual, cursorPos);
                    refresh();

                    currentIndex = (currentIndex + 1) % count; // Próximo índice
                }
            }
        } else if (ch == ' ') { // Tecla Espaço
            if (count > 0) {
                move(linhaAtual + 1, 0);
                printw("Você selecionou a palavra: %s\n", palavra);
                linhaAtual += 2; // Mover para a linha abaixo
                printw("Digite o próximo prefixo (ou '#' para sair):\n");
                refresh();
                memset(prefixo, 0, sizeof(prefixo));
                cursorPos = 0;
                count = 0; // Resetar a contagem
                currentIndex = 0; // Resetar o índice
                move(linhaAtual + 1, 0);
            }
        } else if (ch == '\n') { // Tecla Enter
            if (strcmp(prefixo, "#") == 0) {
                break;
            }
            
            if (modoExclusao) {
                if (palavraExiste(raiz, prefixo)) {
                    removerPalavraArquivo(prefixo);
                    raiz = removerPalavra(raiz, prefixo);
                    move(linhaAtual + 1, 0);
                    printw("Palavra '%s' removida do dicionário.\n", prefixo);
                } else {
                    move(linhaAtual + 1, 0);
                    printw("Palavra '%s' não encontrada no dicionário.\n", prefixo);
                }
                linhaAtual += 2; // Mover para a próxima linha após a mensagem de confirmação
                printw("Digite o próximo prefixo (ou '#' para sair):\n");
                refresh();
                modoExclusao = false;
            } else {
                // Verificar se a palavra já existe na árvore
                if (!palavraExiste(raiz, prefixo)) {
                    adicionarPalavraArquivo(prefixo);
                    inserirPalavra(&raiz, prefixo);
                    move(linhaAtual + 1, 0);
                    printw("Palavra '%s' adicionada ao dicionário.\n", prefixo);
                } else {
                    move(linhaAtual + 1, 0);
                    printw("Palavra '%s' já existe no dicionário.\n", prefixo);
                }
                
                linhaAtual += 2; // Mover para a próxima linha após a mensagem de confirmação
                printw("Digite o próximo prefixo (ou '#' para sair):\n");
                refresh();
            }
            
            memset(prefixo, 0, sizeof(prefixo));
            cursorPos = 0;
            count = 0; // Resetar a contagem
            currentIndex = 0; // Resetar o índice
            move(linhaAtual, 0);
        } else if (ch == 27) { // Tecla ESC
            clear();
            linhaAtual = 1;
            printw("\nAutocompletar cancelado.\n");
            printw("Digite o próximo prefixo (ou '#' para sair):\n");
            refresh();
            memset(prefixo, 0, sizeof(prefixo));
            cursorPos = 0;
            count = 0; // Resetar a contagem
            currentIndex = 0; // Resetar o índice
            move(linhaAtual, 0);
            modoExclusao = false;
        } else if (isprint(ch)) { // Caracter imprimível
            if (strlen(prefixo) < MAX_WORD_LENGTH - 1) {
                prefixo[cursorPos] = ch;
                prefixo[cursorPos + 1] = '\0';
                cursorPos++;
                move(linhaAtual, 0);
                clrtoeol();
                printw("%s", prefixo);
                refresh();
                count = 0; // Resetar a contagem para uma nova busca
                currentIndex = 0; // Resetar o índice para uma nova busca
            }

            if (cursorPos >= 2 && prefixo[cursorPos - 2] == ':' && prefixo[cursorPos - 1] == 'd') {
                modoExclusao = true;
                prefixo[cursorPos - 2] = '\0'; // Remove os últimos 2 caracteres (':d')
                cursorPos -= 2;
                move(linhaAtual, 0);
                clrtoeol();
                printw("%s", prefixo);
                refresh();
            }
        } else if (ch == KEY_BACKSPACE || ch == 127) { // Tecla Backspace
            if (cursorPos > 0) {
                prefixo[cursorPos - 1] = '\0';
                cursorPos--;
                move(linhaAtual, 0);
                clrtoeol();
                printw("%s", prefixo);
                refresh();
                count = 0; // Resetar a contagem para uma nova busca
                currentIndex = 0; // Resetar o índice para uma nova busca
            }
        }
    }

    endwin();
    liberarArvore(raiz);
    return 0;
}

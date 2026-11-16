#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARQUIVO_DOS_CLIENTES "customer.txt"
#define ARQUIVO_DOS_COMANDOS "commands.txt"
#define MAXIMO_TAMANHO_LINHA 1024

#define SOLICITACAO 1
#define LIBERACAO 2
#define ESTADO 3

int number_of_customers;
int number_of_resources;

int *available;
int **maximum;
int **allocation;
int **need;

int converter_texto_para_numero(const char *texto, int *valor){
    char *fim;
    long numero;

    errno = 0;
    numero = strtol(texto, &fim, 10);

    if (fim == texto || *fim != '\0' || errno == ERANGE || numero < 0 || numero > INT_MAX) {
        return -1;
    }

    *valor = (int) numero;
    return 0;
}

int ler_linha(FILE *arquivo, char linha[]){
    
    if (fgets(linha, MAXIMO_TAMANHO_LINHA, arquivo) == NULL) {
        return 0;
    }

    if (strchr(linha, '\n') == NULL && !feof(arquivo)) {
        return -1;
    }

    linha[strcspn(linha, "\r\n")] = '\0';
    return 1;
}

bool linha_vazia(const char *linha){
    while (isspace((unsigned char) *linha)) {
        linha++;
    }
    return *linha == '\0';
}

void liberar_matriz(int **matriz, int linhas){

    if (matriz == NULL) {
        return;
    }

    for (int i = 0; i < linhas; i++) {
        free(matriz[i]);
    }
    free(matriz);
}

int **alocacao_matriz(int linhas, int colunas){
    int **matriz;

    matriz = calloc(linhas, sizeof(int *));
    if (matriz == NULL) {
        return NULL;
    }

    for (int i = 0; i < linhas; i++) {
        matriz[i] = calloc(colunas, sizeof(int));
        if (matriz[i] == NULL) {
            liberar_matriz(matriz, linhas);
            return NULL;
        }
    }

    return matriz;
}

void liberar_memoria(void){
    free(available);
    liberar_matriz(maximum, number_of_customers);
    liberar_matriz(allocation, number_of_customers);
    liberar_matriz(need, number_of_customers);
}

int ler_argumentos(int argc, char *argv[]){

    if (argc < 2) {
        fprintf(stderr, "Entrada no formato errado ou sem algum argumento\n");
        return -1;
    }

    number_of_resources = argc - 1;

    available = malloc(number_of_resources * sizeof(int));

    if (available == NULL) {
        fprintf(stderr, "Sem memoria suficiente\n");
        return -1;
    }

    for (int i = 0; i < number_of_resources; i++) {
        if (converter_texto_para_numero(argv[i + 1], &available[i]) != 0) {
            fprintf(stderr, "Quantidade invalida de instancias\n");
            return -1;
        }
    }

    return 0;
}

int contar_clientes(FILE *arquivo){
    char linha[MAXIMO_TAMANHO_LINHA];
    int numero_linha = 0;
    int status;

    number_of_customers = 0;

    while ((status = ler_linha(arquivo, linha)) == 1) {
        numero_linha++;
        if (!linha_vazia(linha)) {
            number_of_customers++;
        }
    }

    if (status == -1) {
        fprintf(stderr, "Linha eh muito longa\n");
        return -1;
    }

    if (ferror(arquivo)) {
        fprintf(stderr, "Nao foi possivel ler o arquivo\n");
        return -1;
    }

    if (number_of_customers == 0) {
        fprintf(stderr, "Sem clientes\n");
        return -1;
    }

    return 0;
}

int ler_maximo_cliente(char linha[], int cliente, int numero_linha){
    char *token;
    int valor;
    int j = 0;

    token = strtok(linha, ",");
    while (token != NULL && j < number_of_resources) {
        if (converter_texto_para_numero(token, &valor) != 0) {
            fprintf(stderr, "Valor invalido\n");
            return -1;
        }
        if (valor > available[j]) {
            fprintf(stderr, "o maximo do recurso eh maior que o valor total de instancias\n");
            return -1;
        }

        maximum[cliente][j] = valor;
        need[cliente][j] = valor;
        j++;
        token = strtok(NULL, ",");
    }

    if (token != NULL || j != number_of_resources) {
        fprintf(stderr, "eram esperados determinados valores sendo um para cada recurso na execução\n");
        return -1;
    }

    return 0;
}

int ler_clientes(void){
    FILE *arquivo;
    char linha[MAXIMO_TAMANHO_LINHA];
    int numero_linha = 0;
    int cliente = 0;

    arquivo = fopen(ARQUIVO_DOS_CLIENTES, "r");

    if (arquivo == NULL) {
        fprintf(stderr, "Nao foi possivel abrir o arquivo\n");
        return -1;
    }

    if (contar_clientes(arquivo) != 0) {
        fclose(arquivo);
        return -1;
    }

    maximum = alocacao_matriz(number_of_customers, number_of_resources);

    allocation = alocacao_matriz(number_of_customers, number_of_resources);

    need = alocacao_matriz(number_of_customers, number_of_resources);

    if (maximum == NULL || allocation == NULL || need == NULL) {
        fprintf(stderr, "Sem memoria sufuciente\n");
        fclose(arquivo);
        return -1;
    }

    rewind(arquivo);
    while (ler_linha(arquivo, linha) == 1) {
        numero_linha++;
        if (linha_vazia(linha)) {
            continue;
        }
        if (ler_maximo_cliente(linha, cliente, numero_linha) != 0) {
            fclose(arquivo);
            return -1;
        }
        cliente++;
    }

    fclose(arquivo);
    return 0;
}

int entender_comando(char linha[], int numero_linha, int *cliente, int valores[]){
    char *token;
    int tipo;
    int j = 0;

    token = strtok(linha, " \t");

    if (strcmp(token, "*") == 0) {

        if (strtok(NULL, " \t") != NULL) {

            fprintf(stderr, "o comando * não recebe valores\n");
            return -1;
        }
        return ESTADO;
    }

    if (strcmp(token, "RQ") == 0) {
        tipo = SOLICITACAO;

    } else if (strcmp(token, "RL") == 0) {
        tipo = LIBERACAO;

    } else {
        fprintf(stderr, "comando desconhecido\n");
        return -1;
    }

    token = strtok(NULL, " \t");

    if (token == NULL || converter_texto_para_numero(token, cliente) != 0) {
        fprintf(stderr, "numero de cliente invalido ou ausente\n");
        return -1;
    }

    if (*cliente >= number_of_customers) {
        fprintf(stderr, "o cliente não existe\n");
        return -1;
    }

    token = strtok(NULL, " \t");

    while (token != NULL && j < number_of_resources) {
        if (converter_texto_para_numero(token, &valores[j]) != 0) {
            fprintf(stderr, "valor invalido\n");
            return -1;
        }
        j++;
        token = strtok(NULL, " \t");
    }

    if (token != NULL || j != number_of_resources) {
        fprintf(stderr, "rminados valores sendo um para cada recurso na execução\n");
        return -1;
    }

    return tipo;
}

int validar_os_comandos(void){
    FILE *arquivo;
    char linha[MAXIMO_TAMANHO_LINHA];
    int *valores;
    int cliente;
    int numero_linha = 0;
    int status = 0;
    int resultado = 0;

    arquivo = fopen(ARQUIVO_DOS_COMANDOS, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "nao foi possivel abrir o arquivo\n");
        return -1;
    }

    valores = malloc(number_of_resources * sizeof(int));
    if (valores == NULL) {
        fprintf(stderr, "sem memoria sufucicente\n");
        fclose(arquivo);
        return -1;
    }

    while (resultado == 0 && (status = ler_linha(arquivo, linha)) == 1) {
        numero_linha++;
        if (!linha_vazia(linha) && entender_comando(linha, numero_linha, &cliente, valores) == -1) {
            resultado = -1;
        }
    }

    if (status == -1) {
        fprintf(stderr, "linha muito longa\n");
        resultado = -1;
    } else if (resultado == 0 && ferror(arquivo)) {
        fprintf(stderr, "nao foi possivel ler o arquivo\n");
        resultado = -1;
    }

    free(valores);
    fclose(arquivo);
    return resultado;
}

void imprimir_vetor(FILE *saida, int vetor[]){

    for (int j = 0; j < number_of_resources; j++) {
        if (j > 0) {
            fprintf(saida, " ");
        }
        fprintf(saida, "%d", vetor[j]);
    }
}

void imprimir_estado(FILE *saida){

    fprintf(saida, "MAXIMUM | ALLOCATION | NEED\n");
    for (int i = 0; i < number_of_customers; i++) {
        imprimir_vetor(saida, maximum[i]);
        fprintf(saida, " | ");
        imprimir_vetor(saida, allocation[i]);
        fprintf(saida, " | ");
        imprimir_vetor(saida, need[i]);
        fprintf(saida, "\n");
    }

    fprintf(saida, "AVAILABLE ");
    imprimir_vetor(saida, available);
    fprintf(saida, "\n");
}

int main(int argc, char *argv[]){
    
    if (ler_argumentos(argc, argv) != 0 || ler_clientes() != 0 || validar_os_comandos() != 0) {
        liberar_memoria();
        return EXIT_FAILURE;
    }

    imprimir_estado(stdout);

    liberar_memoria();
    return EXIT_SUCCESS;
}
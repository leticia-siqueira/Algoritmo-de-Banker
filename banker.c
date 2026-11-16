#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARQUIVO_DOS_CLIENTES "customer.txt"
#define ARQUIVO_DOS_COMANDOS "commands.txt"
#define ARQUIVO_DO_RESULTADO "result.txt"
#define MAXIMO_TAMANHO_LINHA 1024

#define SOLICITACAO 1
#define LIBERACAO 2
#define ESTADO 3

#define PEDIDO_ACEITO 0
#define PASSOU_DO_MAXIMO -1
#define RECURSOS_INSUFICIENTES -2
#define ESTADO_NAO_SEGURO -3

int number_of_customers;
int number_of_resources;

int *available;
int **maximum;
int **allocation;
int **need;

int *work;
bool *finish;

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

    if (strchr(linha, '\n') == NULL && feof(arquivo) == 0) {
        return -1;
    }

    linha[strcspn(linha, "\r\n")] = '\0';
    return 1;
}

bool linha_vazia(const char *linha){
    while (isspace((unsigned char) *linha) != 0) {
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
    free(work);
    free(finish);
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
    int status;

    number_of_customers = 0;
    while ((status = ler_linha(arquivo, linha)) == 1) {
        if (linha_vazia(linha) == false) {
            number_of_customers++;
        }
    }

    if (status == -1) {
        fprintf(stderr, "%s: linha muito longa\n", ARQUIVO_DOS_CLIENTES);
        return -1;
    }

    if (ferror(arquivo) != 0) {
        fprintf(stderr, "Nao foi possivel ler %s\n", ARQUIVO_DOS_CLIENTES);
        return -1;
    }

    if (number_of_customers == 0) {
        fprintf(stderr, "%s: sem clientes\n", ARQUIVO_DOS_CLIENTES);
        return -1;
    }

    return 0;
}

int ler_maximo_cliente(char linha[], int cliente){
    char *token;
    int valor;
    int j = 0;

    token = strtok(linha, ",");
    while (token != NULL && j < number_of_resources) {
        if (converter_texto_para_numero(token, &valor) != 0) {
            fprintf(stderr, "%s: valor invalido\n", ARQUIVO_DOS_CLIENTES);
            return -1;
        }
        if (valor > available[j]) {
            fprintf(stderr, "%s: maximo maior que o total de instancias\n", ARQUIVO_DOS_CLIENTES);
            return -1;
        }

        maximum[cliente][j] = valor;
        need[cliente][j] = valor;
        j++;
        token = strtok(NULL, ",");
    }

    if (token != NULL || j != number_of_resources) {
        fprintf(stderr, "%s: quantidade de valores diferente da quantidade de recursos\n", ARQUIVO_DOS_CLIENTES);
        return -1;
    }

    return 0;
}

int ler_clientes(void){
    FILE *arquivo;
    char linha[MAXIMO_TAMANHO_LINHA];
    int cliente = 0;

    arquivo = fopen(ARQUIVO_DOS_CLIENTES, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "Nao foi possivel abrir %s\n", ARQUIVO_DOS_CLIENTES);
        return -1;
    }

    if (contar_clientes(arquivo) != 0) {
        fclose(arquivo);
        return -1;
    }

    maximum = alocacao_matriz(number_of_customers, number_of_resources);
    allocation = alocacao_matriz(number_of_customers, number_of_resources);
    need = alocacao_matriz(number_of_customers, number_of_resources);
    work = malloc(number_of_resources * sizeof(int));
    finish = malloc(number_of_customers * sizeof(bool));

    if (maximum == NULL || allocation == NULL || need == NULL || work == NULL || finish == NULL) {
        fprintf(stderr, "Sem memoria suficiente\n");
        fclose(arquivo);
        return -1;
    }

    rewind(arquivo);
    while (ler_linha(arquivo, linha) == 1) {
        if (linha_vazia(linha) == true) {
            continue;
        }
        if (ler_maximo_cliente(linha, cliente) != 0) {
            fclose(arquivo);
            return -1;
        }
        cliente++;
    }

    fclose(arquivo);
    return 0;
}

int entender_comando(char linha[], int *cliente, int valores[]){
    char *token;
    int tipo;
    int j = 0;

    token = strtok(linha, " \t");
    if (strcmp(token, "*") == 0) {
        if (strtok(NULL, " \t") != NULL) {
            fprintf(stderr, "%s: o comando * nao recebe valores\n", ARQUIVO_DOS_COMANDOS);
            return -1;
        }
        return ESTADO;
    }

    if (strcmp(token, "RQ") == 0) {
        tipo = SOLICITACAO;
    } else if (strcmp(token, "RL") == 0) {
        tipo = LIBERACAO;
    } else {
        fprintf(stderr, "%s: comando desconhecido\n", ARQUIVO_DOS_COMANDOS);
        return -1;
    }

    token = strtok(NULL, " \t");
    if (token == NULL || converter_texto_para_numero(token, cliente) != 0) {
        fprintf(stderr, "%s: numero de cliente invalido ou ausente\n", ARQUIVO_DOS_COMANDOS);
        return -1;
    }

    if (*cliente >= number_of_customers) {
        fprintf(stderr, "%s: o cliente nao existe\n", ARQUIVO_DOS_COMANDOS);
        return -1;
    }

    token = strtok(NULL, " \t");
    while (token != NULL && j < number_of_resources) {
        if (converter_texto_para_numero(token, &valores[j]) != 0) {
            fprintf(stderr, "%s: valor invalido\n", ARQUIVO_DOS_COMANDOS);
            return -1;
        }
        j++;
        token = strtok(NULL, " \t");
    }

    if (token != NULL || j != number_of_resources) {
        fprintf(stderr, "%s: quantidade de valores diferente da quantidade de recursos\n", ARQUIVO_DOS_COMANDOS);
        return -1;
    }

    return tipo;
}

int validar_os_comandos(void){
    FILE *arquivo;
    char linha[MAXIMO_TAMANHO_LINHA];
    int *valores;
    int cliente;
    int status = 0;
    int resultado = 0;

    arquivo = fopen(ARQUIVO_DOS_COMANDOS, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "Nao foi possivel abrir %s\n", ARQUIVO_DOS_COMANDOS);
        return -1;
    }

    valores = malloc(number_of_resources * sizeof(int));
    if (valores == NULL) {
        fprintf(stderr, "Sem memoria suficiente\n");
        fclose(arquivo);
        return -1;
    }

    while (resultado == 0 && (status = ler_linha(arquivo, linha)) == 1) {
        if (linha_vazia(linha) == false && entender_comando(linha, &cliente, valores) == -1) {
            resultado = -1;
        }
    }

    if (status == -1) {
        fprintf(stderr, "%s: linha muito longa\n", ARQUIVO_DOS_COMANDOS);
        resultado = -1;
    } else if (resultado == 0 && ferror(arquivo) != 0) {
        fprintf(stderr, "Nao foi possivel ler %s\n", ARQUIVO_DOS_COMANDOS);
        resultado = -1;
    }

    free(valores);
    fclose(arquivo);
    return resultado;
}

bool menor_ou_igual(int x[], int y[]){
    for (int j = 0; j < number_of_resources; j++) {
        if (x[j] > y[j]) {
            return false;
        }
    }
    return true;
}

bool is_safe(void){
    bool encontrou = true;

    for (int j = 0; j < number_of_resources; j++) {
        work[j] = available[j];
    }
    for (int i = 0; i < number_of_customers; i++) {
        finish[i] = false;
    }

    while (encontrou == true) {
        encontrou = false;
        for (int i = 0; i < number_of_customers; i++) {
            if (finish[i] == false && menor_ou_igual(need[i], work) == true) {
                for (int j = 0; j < number_of_resources; j++) {
                    work[j] += allocation[i][j];
                }
                finish[i] = true;
                encontrou = true;
            }
        }
    }

    for (int i = 0; i < number_of_customers; i++) {
        if (finish[i] == false) {
            return false;
        }
    }
    return true;
}

int request_resources(int customer_num, int request[]){
    if (menor_ou_igual(request, need[customer_num]) == false) {
        return PASSOU_DO_MAXIMO;
    }

    if (menor_ou_igual(request, available) == false) {
        return RECURSOS_INSUFICIENTES;
    }

    for (int j = 0; j < number_of_resources; j++) {
        available[j] -= request[j];
        allocation[customer_num][j] += request[j];
        need[customer_num][j] -= request[j];
    }

    if (is_safe() == true) {
        return PEDIDO_ACEITO;
    }

    for (int j = 0; j < number_of_resources; j++) {
        available[j] += request[j];
        allocation[customer_num][j] -= request[j];
        need[customer_num][j] += request[j];
    }
    return ESTADO_NAO_SEGURO;
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
    fprintf(saida, "MAXIMUM   |   ALLOCATION  |   NEED\n");
    for (int i = 0; i < number_of_customers; i++) {
        imprimir_vetor(saida, maximum[i]);
        fprintf(saida, "     |   ");
        imprimir_vetor(saida, allocation[i]);
        fprintf(saida, "       |   ");
        imprimir_vetor(saida, need[i]);
        fprintf(saida, " \n");
    }

    fprintf(saida, "AVAILABLE ");
    imprimir_vetor(saida, available);
    fprintf(saida, " \n");
}

void executar_solicitacao(FILE *saida, int cliente, int request[]){
    switch (request_resources(cliente, request)) {
    case PEDIDO_ACEITO:
        fprintf(saida, "Allocate to customer %d the resources ", cliente);
        imprimir_vetor(saida, request);
        break;
    case RECURSOS_INSUFICIENTES:
        fprintf(saida, "The resources ");
        imprimir_vetor(saida, available);
        fprintf(saida, " are not enough to customer %d request ", cliente);
        imprimir_vetor(saida, request);
        fprintf(saida, " ");
        break;
    case ESTADO_NAO_SEGURO:
        fprintf(saida, "The customer %d request ", cliente);
        imprimir_vetor(saida, request);
        fprintf(saida, " was denied because result in an unsafe state");
        break;
    case PASSOU_DO_MAXIMO:
        fprintf(saida, "The customer %d request ", cliente);
        imprimir_vetor(saida, request);
        fprintf(saida, " was denied because exceed its maximum allocation");
        break;
    }
    fprintf(saida, "\n");
}

int executar_comandos(FILE *saida){
    FILE *arquivo;
    char linha[MAXIMO_TAMANHO_LINHA];
    int *valores;
    int cliente;
    int tipo;

    arquivo = fopen(ARQUIVO_DOS_COMANDOS, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "Nao foi possivel abrir %s\n", ARQUIVO_DOS_COMANDOS);
        return -1;
    }

    valores = malloc(number_of_resources * sizeof(int));
    if (valores == NULL) {
        fprintf(stderr, "Sem memoria suficiente\n");
        fclose(arquivo);
        return -1;
    }

    while (ler_linha(arquivo, linha) == 1) {
        if (linha_vazia(linha) == true) {
            continue;
        }

        tipo = entender_comando(linha, &cliente, valores);
        if (tipo == SOLICITACAO) {
            executar_solicitacao(saida, cliente, valores);
        } else if (tipo == ESTADO) {
            imprimir_estado(saida);
        }
    }

    free(valores);
    fclose(arquivo);
    return 0;
}

int main(int argc, char *argv[]){
    FILE *resultado;
    int status;

    if (ler_argumentos(argc, argv) != 0 || ler_clientes() != 0 || validar_os_comandos() != 0) {
        liberar_memoria();
        return EXIT_FAILURE;
    }

    resultado = fopen(ARQUIVO_DO_RESULTADO, "w");
    if (resultado == NULL) {
        fprintf(stderr, "Nao foi possivel criar %s\n", ARQUIVO_DO_RESULTADO);
        liberar_memoria();
        return EXIT_FAILURE;
    }

    status = executar_comandos(resultado);
    if (fclose(resultado) != 0) {
        fprintf(stderr, "Nao foi possivel gravar %s\n", ARQUIVO_DO_RESULTADO);
        status = -1;
    }

    liberar_memoria();
    if (status != 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
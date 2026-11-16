#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int number_of_resources;

int *available;

int converter_inteiro(const char *texto, int *valor)
{
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

void liberar_memoria(void)
{
    free(available);
}

int ler_argumentos(int argc, char *argv[])
{
    int i;

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <instâncias do recurso 0> <instâncias do recurso 1> ...\n", argv[0]);
        return -1;
    }

    number_of_resources = argc - 1;
    available = malloc(number_of_resources * sizeof(int));
    if (available == NULL) {
        fprintf(stderr, "Erro: memória insuficiente\n");
        return -1;
    }

    for (i = 0; i < number_of_resources; i++) {
        if (converter_inteiro(argv[i + 1], &available[i]) != 0) {
            fprintf(stderr, "Erro: \"%s\" não é uma quantidade válida de instâncias\n", argv[i + 1]);
            return -1;
        }
    }

    return 0;
}

void imprimir_vetor(FILE *saida, int vetor[])
{
    int j;

    for (j = 0; j < number_of_resources; j++) {
        if (j > 0) {
            fprintf(saida, " ");
        }
        fprintf(saida, "%d", vetor[j]);
    }
}

int main(int argc, char *argv[])
{
    if (ler_argumentos(argc, argv) != 0) {
        liberar_memoria();
        return EXIT_FAILURE;
    }

    printf("AVAILABLE ");
    imprimir_vetor(stdout, available);
    printf("\n");

    liberar_memoria();
    return EXIT_SUCCESS;
}
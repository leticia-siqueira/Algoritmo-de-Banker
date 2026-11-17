# Implementação 5 - Algoritmo do Banqueiro

## Sistema operacional

Ubuntu/WSL

## Arquivos

| Arquivo | Descrição |
| --- | --- |
| `banker.c` | Programa completo. Lê e valida os argumentos, o `customer.txt` e o `commands.txt`; executa o algoritmo de segurança, as solicitações (`RQ`) e as liberações (`RL`); escreve o resultado de cada comando e o estado do sistema (`*`) em `result.txt`. |
| `Makefile` | `make` compila o `banker.c` e gera o executável `banker`; `make clean` remove o executável. |
| `evidencias.log` | Registro das sessões de terminal com a compilação e os testes, gravado com `script`. |

O programa também usa dois arquivos de entrada, que precisam estar na pasta onde ele é executado:

- `customer.txt`: uma linha por cliente, com a demanda máxima de cada recurso separada por vírgulas. O número de linhas define o número de clientes.

  ```
  7,5,3
  3,2,2
  9,0,2
  2,2,2
  4,3,3
  ```

- `commands.txt`: um comando por linha.
  - `RQ <cliente> <recursos>`: solicita recursos.
  - `RL <cliente> <recursos>`: libera recursos.
  - `*`: grava o estado atual (MAXIMUM, ALLOCATION, NEED e AVAILABLE).

  ```
  RQ 0 0 2 1
  RL 0 0 1 1
  *
  ```

## Como compilar

```
make clean
make
```

## Como executar

Cada argumento é o número de instâncias de um tipo de recurso. Para três recursos com 10, 5 e 7 instâncias:

```
./banker 10 5 7
```

A quantidade de argumentos precisa ser igual à quantidade de valores de cada linha do `customer.txt` e dos comandos `RQ`/`RL`. A saída fica em `result.txt`, que é recriado a cada execução.

Se algum argumento ou arquivo estiver errado (arquivo inexistente ou sem permissão, valor inválido, quantidade de valores diferente da quantidade de recursos, comando desconhecido, cliente inexistente), o programa mostra uma mensagem no terminal, não executa nenhum comando e termina com código 1.

## Como testar

1. Coloque na pasta o `customer.txt` e o `commands.txt` disponibilizados pelo professor.
2. Compile e execute:

   ```
   make
   ./banker 10 5 7
   ```


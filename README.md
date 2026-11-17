# Implementação 5 - Algoritmo do Banqueiro

Implementação em C do algoritmo do Banqueiro (Banker's Algorithm) descrito na seção 8.6.3 do livro *Operating System Concepts* (Silberschatz, Galvin e Gagne, 10ª edição), com as modificações pedidas na atividade: o número de clientes e de recursos vem da entrada, os comandos são lidos de um arquivo e a saída é gravada em `result.txt`.

## Sistema operacional

Implementado e testado em **[PREENCHER: sistema operacional e versão]**, com `gcc` e `make`. O código usa apenas C padrão (C99), então compila também em outros sistemas Linux, Unix e macOS.

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

1. Coloque na pasta o `customer.txt` e o `commands.txt` disponibilizados pelo professor e salve o `result.txt` dele com outro nome, por exemplo `esperado.txt`.
2. Compile e execute:

   ```
   make
   ./banker 10 5 7
   ```

3. Compare a saída com a esperada:

   ```
   diff result.txt esperado.txt
   ```

   A única diferença esperada é o aviso de quebra de linha no fim do arquivo (`\ No newline at end of file`), porque o `result.txt` gerado termina com quebra de linha.

Alguns casos de erro para conferir o tratamento de entradas inválidas:

| Comando ou entrada | Mensagem esperada |
| --- | --- |
| `./banker` | `Entrada no formato errado ou sem algum argumento` |
| `./banker 10 abc 7` | `Quantidade invalida de instancias` |
| `./banker 10 5` (com o `customer.txt` acima) | `customer.txt: quantidade de valores diferente da quantidade de recursos` |
| linha `XX 0 1 1 1` no `commands.txt` | `commands.txt: comando desconhecido` |
| linha `RQ 9 0 0 0` no `commands.txt` | `commands.txt: o cliente nao existe` |
| `customer.txt` ausente | `Nao foi possivel abrir customer.txt` |

Depois de cada execução, `echo $?` mostra `0` quando deu certo e `1` quando houve erro.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATH 100
#define MAX_NOME 30

typedef struct _mov *ptrMovimentacao;
typedef struct _mov {
    short int tipo;
    int idClienteDest;
    int idClienteOrig;
    float valor;
    ptrMovimentacao prox;
} Movimentacao;

typedef struct _conta {
    int numero;
    float saldo;
    ptrMovimentacao movimentacoes;
} Conta;

typedef struct _cliente *ptrCliente;
typedef struct _cliente {
    int id;
    Conta conta;
    ptrCliente prox;
} Cliente;

typedef struct _banco {
    char nome[MAX_NOME];
    ptrCliente clientes;
} Banco;

ptrMovimentacao criarNovaMovimentacao(short int tipo, float valor, int idClienteOrig, int idClienteDest) {
    ptrMovimentacao novaMov = (ptrMovimentacao)malloc(sizeof(Movimentacao));
    novaMov->tipo = tipo;
    novaMov->idClienteOrig = idClienteOrig;
    novaMov->idClienteDest = idClienteDest;
    novaMov->valor = valor;
    novaMov->prox = NULL;
    return novaMov;
}

Cliente* buscarCliente(Banco *b, int idCliente) {
    ptrCliente atual = b->clientes;
    while (atual != NULL) {
        if (atual->id == idCliente) {
            return atual;
        }
        atual = atual->prox;
    }
    return NULL;
}

void realizarDeposito(Cliente *c, Movimentacao* dep) {
    if (dep != NULL) {
        c->conta.saldo += dep->valor;
        dep->prox = c->conta.movimentacoes;
        c->conta.movimentacoes = dep;
    }
}

void realizarSaque(Cliente *c, Movimentacao* saque) {
    if (saque != NULL) {
        c->conta.saldo -= saque->valor;
        saque->prox = c->conta.movimentacoes;
        c->conta.movimentacoes = saque;
    }
}

void realizarTransferencia(Cliente *clienteOrig, Cliente *clienteDest, float valor) {
    Movimentacao *movOrig = criarNovaMovimentacao(1, valor, clienteOrig->id, 0);
    Movimentacao *movDest = criarNovaMovimentacao(0, valor, clienteDest->id, 0);

    if (movOrig != NULL && movDest != NULL) {
        clienteOrig->conta.saldo -= valor;
        clienteDest->conta.saldo += valor;

        movOrig->prox = clienteOrig->conta.movimentacoes;
        clienteOrig->conta.movimentacoes = movOrig;

        movDest->prox = clienteDest->conta.movimentacoes;
        clienteDest->conta.movimentacoes = movDest;
    }
}

void adicionarCliente(Banco *b, Cliente *c) {
    if (c != NULL) {
        ptrCliente atual = b->clientes;
        ptrCliente anterior = NULL;

        while (atual != NULL && atual->id < c->id) {
            anterior = atual;
            atual = atual->prox;
        }

        if (anterior == NULL) {
            c->prox = b->clientes;
            b->clientes = c;
        } else {
            c->prox = anterior->prox;
            anterior->prox = c;
        }
    }
}

ptrCliente criarNovoCliente(int idCliente, int numConta, float saldo) {
    ptrCliente novoCliente = (ptrCliente)malloc(sizeof(Cliente));
    if (novoCliente != NULL) {
        novoCliente->id = idCliente;
        novoCliente->conta.numero = numConta;
        novoCliente->conta.saldo = saldo;
        novoCliente->conta.movimentacoes = NULL;
        novoCliente->prox = NULL;
    }
    return novoCliente;
}

Banco* criarBanco(char *nome) {
    Banco *novoBanco = (Banco *)malloc(sizeof(Banco));
    if (novoBanco != NULL) {
        strcpy(novoBanco->nome, nome);
        novoBanco->clientes = NULL;
    }
    return novoBanco;
}

void liberarBanco(Banco *b) {
    ptrCliente atual = b->clientes;
    while (atual != NULL) {
        ptrCliente proximo = atual->prox;
        ptrMovimentacao atualMov = atual->conta.movimentacoes;
        while (atualMov != NULL) {
            ptrMovimentacao proxMov = atualMov->prox;
            free(atualMov);
            atualMov = proxMov;
        }
        free(atual);
        atual = proximo;
    }
    free(b);
}

void imprimirDados(Banco *B) {
    ptrCliente atual = B->clientes;
    while (atual != NULL) {
        printf("=====================================================\n");
        printf("Id. Cliente : %d\nNumero Conta : %d\nSaldo inicial : %.2f\n",
               atual->id, atual->conta.numero, atual->conta.saldo);
        printf("- - - - - - - - - - - - - - - - - - - Movimentacoes - - - - - - - - - - - - - - - - - - -\n");

        ptrMovimentacao mov = atual->conta.movimentacoes;
        while (mov != NULL) {
            printf("Tipo: %s | Valor: %.2f",
                   mov->tipo == 0 ? "Deposito" : (mov->tipo == 1 ? "Saque" : "Transf."),
                   mov->valor);
            if (mov->tipo == 2) {
                printf(" ===> %s: %d\n", mov->idClienteOrig == atual->id ? "Destinatario" : "Origem",
                       mov->idClienteOrig == atual->id ? mov->idClienteDest : mov->idClienteOrig);
            } else {
                printf("\n");
            }
            mov = mov->prox;
        }

        printf("Saldo Final: %.2f\n", atual->conta.saldo);
        printf("=====================================================\n");
        atual = atual->prox;
    }
}

void readFile(FILE *ptr, Banco *banco) {
    int idCliente, numConta, tipo;
    float saldo, valor;
    while (fscanf(ptr, "%d %d %f", &idCliente, &numConta, &saldo) == 3) {
        ptrCliente novoCliente = criarNovoCliente(idCliente, numConta, saldo);
        if (novoCliente != NULL) {
            adicionarCliente(banco, novoCliente);
            while (fscanf(ptr, "%d %f %d %f", &tipo, &valor, &idCliente, &saldo) == 4) {
                if (tipo == 0) {
                    Movimentacao *mov = criarNovaMovimentacao(0, valor, novoCliente->id, 0);
                    if (mov != NULL) {
                        realizarDeposito(novoCliente, mov);
                    }
                } else if (tipo == 1) {
                    Movimentacao *mov = criarNovaMovimentacao(1, valor, novoCliente->id, 0);
                    if (mov != NULL) {
                        realizarSaque(novoCliente, mov);
                    }
                } else if (tipo == 2) {
                    Cliente *destinatario = buscarCliente(banco, idCliente);
                    if (destinatario != NULL) {
                        realizarTransferencia(novoCliente, destinatario, valor);
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    char path[MAX_PATH];
    FILE *filePtr = NULL;
    Banco *bomBanco = NULL;

    scanf("%s", path);
    filePtr = fopen(path, "r");

    if (filePtr) {
        bomBanco = criarBanco("BomBanco");
        if (bomBanco) {
            readFile(filePtr, bomBanco);
            imprimirDados(bomBanco);
            liberarBanco(bomBanco);
        }
        fclose(filePtr);
    } else {
        printf("Falha ao tentar abrir o arquivo\n");
        exit(1);
    }
    return 0;
}

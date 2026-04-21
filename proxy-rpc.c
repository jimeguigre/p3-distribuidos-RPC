#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "claves.h"
#include "clavesRPC.h"

static CLIENT *conectar_rpc() {
    char *ip_servidor = getenv("IP_TUPLAS");
    if (ip_servidor == NULL) return NULL;
    CLIENT *clnt = clnt_create(ip_servidor, CLAVES_PROG, CLAVES_VERS, "tcp");
    return clnt;
}

int destroy() {
    CLIENT *clnt = conectar_rpc();
    if (clnt == NULL) return -1;

    int res; // Variable para el resultado
    enum clnt_stat status = destroy_1(&res, clnt);

    if (status != RPC_SUCCESS) {
        clnt_destroy(clnt);
        return -1;
    }

    clnt_destroy(clnt);
    return res;
}

int set_value(char *key, char *value1, int n_value2, float *v_value2, struct Paquete value3) {
    CLIENT *clnt = conectar_rpc();
    if (clnt == NULL) return -1;

    struct set_value_args args;
    args.key = key;
    args.value1 = value1;
    args.n_value2 = n_value2;
    for (int i = 0; i < n_value2; i++) args.v_value2[i] = v_value2[i];
    args.value3.x = value3.x;
    args.value3.y = value3.y;
    args.value3.z = value3.z;

    int res;
    // IMPORTANTE: Se pasa 'args' (no &args) y '&res'
    enum clnt_stat status = set_value_1(args, &res, clnt);

    if (status != RPC_SUCCESS) {
        clnt_perror(clnt, "Fallo en la llamada RPC");
        clnt_destroy(clnt);
        return -1;
    }

    if (res == -1) {
    printf("El servidor rechazó la inserción (posible clave duplicada o N_value2 fuera de rango)\n");
    }

    clnt_destroy(clnt);
    return res;
}

int get_value(char *key, char *value1, int *n_value2, float *v_value2, struct Paquete *value3) {
    CLIENT *clnt = conectar_rpc();
    if (clnt == NULL) return -1;

    struct get_value_res res_rpc;

    // IMPORTANTE: Inicializar a cero para que los punteros internos de res_rpc sean NULL
    memset(&res_rpc, 0, sizeof(res_rpc));

    // Se pasa 'key' y '&res_rpc'
    enum clnt_stat status = get_value_1(key, &res_rpc, clnt);

    if (status != RPC_SUCCESS) {
        clnt_destroy(clnt);
        return -1;
    }

    if (res_rpc.resultado == 0) {
        // 1. Copiar el string (XDR reserva memoria en res_rpc.value1 automáticamente)
        if (res_rpc.value1 != NULL) {
            strncpy(value1, res_rpc.value1, 255);
            value1[255] = '\0';
        }

        // 2. Copiar los valores numéricos
        *n_value2 = res_rpc.n_value2;
        for (int i = 0; i < res_rpc.n_value2; i++) {
            v_value2[i] = res_rpc.v_value2[i];
        }

        value3->x = res_rpc.value3.x;
        value3->y = res_rpc.value3.y;
        value3->z = res_rpc.value3.z;
    }

    int final_res = res_rpc.resultado;
    xdr_free((xdrproc_t)xdr_get_value_res, (char *)&res_rpc);
    clnt_destroy(clnt);
    return final_res;
}

int modify_value(char *key, char *value1, int n_value2, float *v_value2, struct Paquete value3) {
    CLIENT *clnt = conectar_rpc();
    if (clnt == NULL) return -1;

    struct set_value_args args;
    args.key = key;
    args.value1 = value1;
    args.n_value2 = n_value2;
    for (int i = 0; i < n_value2; i++) args.v_value2[i] = v_value2[i];
    args.value3.x = value3.x;
    args.value3.y = value3.y;
    args.value3.z = value3.z;

    int res;
    enum clnt_stat status = modify_value_1(args, &res, clnt);

    if (status != RPC_SUCCESS) {
        clnt_destroy(clnt);
        return -1;
    }

    clnt_destroy(clnt);
    return res;
}

int delete_key(char *key) {
    CLIENT *clnt = conectar_rpc();
    if (clnt == NULL) return -1;

    int res;
    enum clnt_stat status = delete_key_1(key, &res, clnt);

    if (status != RPC_SUCCESS) {
        clnt_destroy(clnt);
        return -1;
    }

    clnt_destroy(clnt);
    return res;
}

int exist(char *key) {
    CLIENT *clnt = conectar_rpc();
    if (clnt == NULL) return -1;

    int res;
    enum clnt_stat status = exist_1(key, &res, clnt);

    if (status != RPC_SUCCESS) {
        clnt_destroy(clnt);
        return -1;
    }

    clnt_destroy(clnt);
    return res;
}
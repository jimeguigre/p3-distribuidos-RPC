#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "claves.h"
#include "clavesRPC.h"

// Función auxiliar para conectar al servidor RPC usando la IP del entorno
static CLIENT *conectar_rpc() {
    char *ip_servidor = getenv("IP_TUPLAS");
    if (ip_servidor == NULL) return NULL;
    // se utiliza el protocolo de transporte TCP para mayor fiabilidad en la comunicación entre el cliente y el servidor RPC
    CLIENT *clnt = clnt_create(ip_servidor, CLAVES_PROG, CLAVES_VERS, "tcp");
    return clnt;
}

// Cada función del proxy sigue la lógica de:
// 1. Conectarse al servidor RPC.
// 2. Preparar los argumentos (si es necesario).
// 3. Llamar a la función RPC correspondiente.
// 4. Manejar el resultado y errores.

int destroy() {
    CLIENT *clnt = conectar_rpc();
    if (clnt == NULL) return -1;

    int res; // Variable para el resultado
    // invocación al sub del cliente (suplente) geenerado por rpcgen 
    enum clnt_stat status = destroy_1(&res, clnt);

    if (status != RPC_SUCCESS) {
        clnt_destroy(clnt); // liberación del manejador RPC en caso de fallo en la llamada
        return -1;
    }

    clnt_destroy(clnt);
    return res;
}

int set_value(char *key, char *value1, int n_value2, float *V_value2, struct Paquete value3) {
    CLIENT *clnt = conectar_rpc();
    if (clnt == NULL) return -1;

    // encapsulamos los argumentos en la estructura generada por rpcgen para set_value
    struct set_value_args args;
    args.key = key;
    strncpy(args.value1, value1, 255);
    args.value1[255] = '\0'; // Aseguramos el cierre de la cadena
    args.N_value2 = n_value2;
    for (int i = 0; i < n_value2; i++) args.V_value2[i] = V_value2[i];
    args.value3.x = value3.x;
    args.value3.y = value3.y;
    args.value3.z = value3.z;

    int res;
    // la biblioteca RPC se encarga de convertir los datos al estándar XDR.
    enum clnt_stat status = set_value_1(args, &res, clnt);

    if (status != RPC_SUCCESS) {
        clnt_perror(clnt, "Fallo en la llamada RPC"); // indica error de red o timeout
        clnt_destroy(clnt);
        return -1;
    }

    if (res == -1) {
    printf("El servidor rechazó la inserción (posible clave duplicada o N_value2 fuera de rango)\n");
    }

    clnt_destroy(clnt);
    return res;
}

int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {
    CLIENT *clnt = conectar_rpc();
    if (clnt == NULL) return -1;

    struct get_value_res res_rpc;

    // Inicializar a cero para que los punteros internos de res_rpc sean NULL
    memset(&res_rpc, 0, sizeof(res_rpc));

    // Se pasa 'key' y '&res_rpc'
    enum clnt_stat status = get_value_1(key, &res_rpc, clnt);

    if (status != RPC_SUCCESS) {
        clnt_destroy(clnt);
        return -1;
    }

    if (res_rpc.resultado == 0) {
        // copia del string (XDR reserva memoria en res_rpc.value1 automáticamente)
        /*if (res_rpc.value1 != NULL) {
            strncpy(value1, res_rpc.value1, 255);
            value1[255] = '\0';
        }*/

        // copia de los valores numéricos
        *N_value2 = res_rpc.N_value2;
        for (int i = 0; i < res_rpc.N_value2; i++) {
            V_value2[i] = res_rpc.V_value2[i];
        }

        value3->x = res_rpc.value3.x;
        value3->y = res_rpc.value3.y;
        value3->z = res_rpc.value3.z;
    }

    // guardamos el resultado antes de liberar la memoria, ya que xdr_free liberará los punteros internos de res_rpc
    int final_res = res_rpc.resultado;
    xdr_free((xdrproc_t)xdr_get_value_res, (char *)&res_rpc);
    clnt_destroy(clnt);
    return final_res;
}

// La función modify_value sigue una lógica similar a set_value, pero con la estructura de argumentos específica para esta operación.
int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    CLIENT *clnt = conectar_rpc();
    if (clnt == NULL) return -1;

    // encapsulamos los argumentos en la estructura generada por rpcgen para modify_value
    struct set_value_args args;
    args.key = key;
    strncpy(args.value1, value1, 255);
    args.value1[255] = '\0'; // Aseguramos el cierre de la cadena
    args.N_value2 = N_value2;
    for (int i = 0; i < N_value2; i++) args.V_value2[i] = V_value2[i];
    args.value3.x = value3.x;
    args.value3.y = value3.y;
    args.value3.z = value3.z;

    int res;
    // la biblioteca RPC se encarga de convertir los datos al estándar XDR.
    enum clnt_stat status = modify_value_1(args, &res, clnt);

    // en caso de fallo en la llamada RPC, se devuelve -1
    if (status != RPC_SUCCESS) {
        clnt_destroy(clnt); // liberación del manejador RPC en caso de fallo en la llamada
        return -1;
    }

    clnt_destroy(clnt);
    return res;
}

// Las funciones delete_key y exist siguen una lógica similar a destroy, pero con la estructura de argumentos específica para cada operación.
int delete_key(char *key) {
    CLIENT *clnt = conectar_rpc();
    if (clnt == NULL) return -1;

    int res;
    // la biblioteca RPC se encarga de convertir los datos al estándar XDR.
    enum clnt_stat status = delete_key_1(key, &res, clnt);

    // en caso de fallo en la llamada RPC, se devuelve -1
    if (status != RPC_SUCCESS) {
        clnt_destroy(clnt); // liberación del manejador RPC en caso de fallo en la llamada
        return -1;
    }

    clnt_destroy(clnt);
    return res;
}

// La función exist devuelve 1 si la clave existe, 0 si no existe y -1 en caso de error
int exist(char *key) {
    CLIENT *clnt = conectar_rpc();
    if (clnt == NULL) return -1;

    int res;
    // la biblioteca RPC se encarga de convertir los datos al estándar XDR.
    enum clnt_stat status = exist_1(key, &res, clnt);

    // en caso de fallo en la llamada RPC, se devuelve -1
    if (status != RPC_SUCCESS) {
        clnt_destroy(clnt); // liberación del manejador RPC en caso de fallo en la llamada 
        return -1;
    }

    clnt_destroy(clnt);
    return res;
}
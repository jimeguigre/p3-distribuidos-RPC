#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "claves.h"

//Estructura para pasar el socket al hilo
typedef struct {
    int client_sock;
} ThreadArgs;

// FUNCIONES AUXILIARES DE ENVÍO Y RECEPCIÓN COMPLETA
static ssize_t send_todo(int sock, const void *buf, size_t len) {
    size_t enviado = 0;
    while (enviado < len) {
        ssize_t s = send(sock, (const char *)buf + enviado, len - enviado, 0);
        if (s <= 0) return -1;
        enviado += s;
    }
    return (ssize_t)enviado;
}

static ssize_t recv_todo(int sock, void *buf, size_t len) {
    size_t recibido = 0;
    while (recibido < len) {
        ssize_t r = recv(sock, (char *)buf + recibido, len - recibido, 0);
        if (r <= 0) return -1;
        recibido += r;
    }
    return (ssize_t)recibido;
}


// Conversión de float a/desde orden de red
static uint32_t float_a_red(float f) {
    uint32_t tmp;
    memcpy(&tmp, &f, sizeof(float));
    return htonl(tmp);
}

static float red_a_float(uint32_t red) {
    uint32_t tmp = ntohl(red);
    float f;
    memcpy(&f, &tmp, sizeof(float));
    return f;
}

//Hilo que procesa la petición de un cliente
void *tratar_peticion(void *args) {
    ThreadArgs *targs = (ThreadArgs *)args;
    int client_sock = targs->client_sock;
    free(targs); //Libera memoria alojada dinámicamente

    int op_red;
    if (recv_todo(client_sock, &op_red, sizeof(int)) < 0) {
        close(client_sock);
        pthread_exit(NULL);
    }
    int op = ntohl(op_red);
    int resultado = -1;

    if (op == 0) { //SET_VALUE
        char key[256] = {0};
        char val1[256] = {0};
        int n2_red;
        float v2[32];
        struct Paquete p;

        if (recv_todo(client_sock, key,      256)         < 0) goto fin;
        if (recv_todo(client_sock, val1,     256)         < 0) goto fin;
        if (recv_todo(client_sock, &n2_red,  sizeof(int)) < 0) goto fin;

        int n2 = ntohl(n2_red);

        // Recibir cada float desde orden de red
        for (int i = 0; i < n2; i++) {
            uint32_t f_red;
            if (recv_todo(client_sock, &f_red, sizeof(uint32_t)) < 0) goto fin;
            v2[i] = red_a_float(f_red);
        }

        int x_red, y_red, z_red;
        if (recv_todo(client_sock, &x_red, sizeof(int)) < 0) goto fin;
        if (recv_todo(client_sock, &y_red, sizeof(int)) < 0) goto fin;
        if (recv_todo(client_sock, &z_red, sizeof(int)) < 0) goto fin;
        p.x = ntohl(x_red); p.y = ntohl(y_red); p.z = ntohl(z_red);

        resultado = set_value(key, val1, n2, v2, p);

        int res_red = htonl(resultado);
        send_todo(client_sock, &res_red, sizeof(int));

        } else if (op == 1) { //GET_VALUE
            char key[256] = {0};
            char val1[256] = {0};
            int n2;
            float v2[32];
            struct Paquete p;

            if (recv_todo(client_sock, key, 256) < 0) goto fin;

            resultado = get_value(key, val1, &n2, v2, &p);

            int res_red = htonl(resultado);
            if (send_todo(client_sock, &res_red, sizeof(int)) < 0) goto fin;

            if (resultado == 0) {
                if (send_todo(client_sock, val1, 256) < 0) goto fin;

                int n2_net = htonl(n2);
                if (send_todo(client_sock, &n2_net, sizeof(int)) < 0) goto fin;

                // Enviar cada float convertido a orden de red
                for (int i = 0; i < n2; i++) {
                    uint32_t f_red = float_a_red(v2[i]);
                    if (send_todo(client_sock, &f_red, sizeof(uint32_t)) < 0) goto fin;
                }

                int x_net = htonl(p.x);
                int y_net = htonl(p.y);
                int z_net = htonl(p.z);
                if (send_todo(client_sock, &x_net, sizeof(int)) < 0) goto fin;
                if (send_todo(client_sock, &y_net, sizeof(int)) < 0) goto fin;
                if (send_todo(client_sock, &z_net, sizeof(int)) < 0) goto fin;
            }

        } else if (op == 2) { //MODIFY_VALUE
            char key[256] = {0};
            char val1[256] = {0};
            int n2_red;
            float v2[32];
            struct Paquete p;

            if (recv_todo(client_sock, key,     256)         < 0) goto fin;
            if (recv_todo(client_sock, val1,    256)         < 0) goto fin;
            if (recv_todo(client_sock, &n2_red, sizeof(int)) < 0) goto fin;

            int n2 = ntohl(n2_red);

            // Recibir cada float desde orden de red
            for (int i = 0; i < n2; i++) {
                uint32_t f_red;
                if (recv_todo(client_sock, &f_red, sizeof(uint32_t)) < 0) goto fin;
                v2[i] = red_a_float(f_red);
            }

            int x_red, y_red, z_red;
            if (recv_todo(client_sock, &x_red, sizeof(int)) < 0) goto fin;
            if (recv_todo(client_sock, &y_red, sizeof(int)) < 0) goto fin;
            if (recv_todo(client_sock, &z_red, sizeof(int)) < 0) goto fin;
            p.x = ntohl(x_red); p.y = ntohl(y_red); p.z = ntohl(z_red);

            resultado = modify_value(key, val1, n2, v2, p);

            int res_red = htonl(resultado);
            send_todo(client_sock, &res_red, sizeof(int));

        } else if (op == 3) { //DELETE_KEY
            char key[256] = {0};
            if (recv_todo(client_sock, key, 256) < 0) goto fin;

            resultado = delete_key(key);

            int res_red = htonl(resultado);
            send_todo(client_sock, &res_red, sizeof(int));

        } else if (op == 4) { //EXIST
            char key[256] = {0};
            if (recv_todo(client_sock, key, 256) < 0) goto fin;

            resultado = exist(key);

            int res_red = htonl(resultado);
            send_todo(client_sock, &res_red, sizeof(int));

        } else if (op == 5) { // DESTROY
            resultado = destroy();

            int res_red = htonl(resultado);
            send_todo(client_sock, &res_red, sizeof(int));
        }

fin:
    close(client_sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <PUERTO>\n", argv[0]);
        return -1;
    }

    int port = atoi(argv[1]);
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Error al crear socket");
        return -1;
    }
    
    //Permite reutilizar el puerto instantáneamente tras cerrar el servidor
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error en bind");
        return -1;
    }

    listen(server_sock, 100);
    printf("Servidor TCP escuchando en puerto %d...\n", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);

        if (client_sock >= 0) {
            pthread_t thread_id;
            ThreadArgs *args = malloc(sizeof(ThreadArgs));
            args->client_sock = client_sock;
            
            pthread_create(&thread_id, NULL, tratar_peticion, (void *)args);
            pthread_detach(thread_id);
        }
    }

    close(server_sock);
    return 0;
}
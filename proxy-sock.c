#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <netdb.h>
#include "claves.h"


// FUNCIONES AUXILIARES: Enviar y recibir todo el buffer
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

// FUNCIÓN AUXILIAR: Establecer la conexión TCP con el servidor
// Lee IP_TUPLAS y PORT_TUPLAS del entorno.

static int conectar_servidor() {
    char *ip_str = getenv("IP_TUPLAS");
    char *port_str = getenv("PORT_TUPLAS");

    if (ip_str == NULL || port_str == NULL) {
        printf("ERROR: Variables de entorno IP_TUPLAS o PORT_TUPLAS no definidas.\n");
        return -1;
    }

    int port = atoi(port_str);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(port);
 
    struct hostent *he = gethostbyname(ip_str);
    if (he == NULL) {
        close(sock);
        return -1;
    }
    memcpy(&server_addr.sin_addr, he->h_addr_list[0], he->h_length);
 
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        return -1;
    }
    return sock;
}


// IMPLEMENTACIÓN DE LA API


int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    // Validaciones básicas
    if (key == NULL || value1 == NULL || V_value2 == NULL) return -1;
    if (N_value2 < 1 || N_value2 > 32)  return -1;
    if (strlen(value1) > 255)            return -1;
 
    int sock = conectar_servidor();
    if (sock < 0) return -1;

    //Opcode: 0 para set_value
    int op = htonl(0);
    if (send_todo(sock, &op, sizeof(int)) < 0) { 
        close(sock); 
        return -1; 
    }
    
    //Key (tamaño fijo 256 para simplificar protocolo)
    char key_buf[256] = {0};
    strncpy(key_buf, key, 255);
    if (send_todo(sock, key_buf, 256) < 0) { 
        close(sock); 
        return -1; 
    }

    //Value1
    char val1_buf[256] = {0};
    strncpy(val1_buf, value1, 255);
    if (send_todo(sock, val1_buf, 256) < 0) { 
        close(sock); 
        return -1; 
    }

    //N_value2
    int net_N2 = htonl(N_value2);
    if (send_todo(sock, &net_N2, sizeof(int)) < 0) { 
        close(sock); 
        return -1; 
    }

    // V_value2: cada float convertido a orden de red
    for (int i = 0; i < N_value2; i++) {
        uint32_t f_red = float_a_red(V_value2[i]);
        if (send_todo(sock, &f_red, sizeof(uint32_t)) < 0) { 
            close(sock);
            return -1;
        }

    }

    //value3 (struct desglosado)
    int net_x = htonl(value3.x);
    int net_y = htonl(value3.y);
    int net_z = htonl(value3.z);
    if (send_todo(sock, &net_x, sizeof(int)) < 0) { 
        close(sock); 
        return -1; 
    }
    if (send_todo(sock, &net_y, sizeof(int)) < 0) { 
        close(sock); 
        return -1; 
    }
    if (send_todo(sock, &net_z, sizeof(int)) < 0) { 
        close(sock); 
        return -1; 
    }

    //Recibir resultado
    int resultado_red;
    if (recv_todo(sock, &resultado_red, sizeof(int)) < 0) {
        close(sock);
        return -1;
    }

    close(sock);
    return ntohl(resultado_red);
}

int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {
    if (key == NULL || value1 == NULL || N_value2 == NULL || V_value2 == NULL || value3 == NULL)
        return -1;
    int sock = conectar_servidor();

    if (sock < 0) return -1;

    //Opcode: 1 para get_value
    int op = htonl(1);
    if (send_todo(sock, &op, sizeof(int)) < 0) { 
        close(sock); 
        return -1; 
    }

    //Key
    char key_buf[256] = {0};
    strncpy(key_buf, key, 255);
    if (send_todo(sock, key_buf, 256) < 0) { 
        close(sock); 
        return -1; 
    }

    //Recibir resultado de la operación
    int res_red;
    if (recv_todo(sock, &res_red, sizeof(int)) < 0) { 
        close(sock); 
        return -1; 
    }
    int res = ntohl(res_red);

    //Si existe, recibimos el resto de datos en el mismo orden
    if (res == 0) {
        // value1
        if (recv_todo(sock, value1, 256) < 0) { 
            close(sock); 
            return -1; 
        }
        
        // N_value2
        int n2_red;
        if (recv_todo(sock, &n2_red, sizeof(int)) < 0) {
            close(sock);
            return -1;
        }
        *N_value2 = ntohl(n2_red);
        
        // V_value2: cada float desde orden de red
        for (int i = 0; i < *N_value2; i++) {
            uint32_t f_red;
            if (recv_todo(sock, &f_red, sizeof(uint32_t)) < 0) { 
                close(sock); 
                return -1; 
            }
            V_value2[i] = red_a_float(f_red);
        }

        // value3
        int x_red, y_red, z_red;
        if (recv_todo(sock, &x_red, sizeof(int)) < 0) { 
            close(sock); 
            return -1; 
        }
        if (recv_todo(sock, &y_red, sizeof(int)) < 0) { 
            close(sock); 
            return -1; 
        }
        if (recv_todo(sock, &z_red, sizeof(int)) < 0) { 
            close(sock); 
            return -1; 
        }
        value3->x = ntohl(x_red);
        value3->y = ntohl(y_red);
        value3->z = ntohl(z_red);
    }

    close(sock);
    return res;
}

int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    if (key == NULL || value1 == NULL || V_value2 == NULL) return -1;
    if (N_value2 < 1 || N_value2 > 32)  return -1;
    if (strlen(value1) > 255)            return -1;

    int sock = conectar_servidor();
    if (sock < 0) return -1;

    //Opcode: 2 para modify_value
    int op = htonl(2);
     if (send_todo(sock, &op, sizeof(int)) < 0) { 
        close(sock); 
        return -1; 
    }

    //key 
    char key_buf[256] = {0};
    strncpy(key_buf, key, 255);
    if (send_todo(sock, key_buf, 256) < 0) { 
        close(sock); 
        return -1; 
    }

    //value1
    char val1_buf[256] = {0};
    strncpy(val1_buf, value1, 255);
    if (send_todo(sock, val1_buf, 256) < 0) { 
        close(sock); 
        return -1; 
    }

    // N_value2
    int net_N2 = htonl(N_value2);
    if (send_todo(sock, &net_N2, sizeof(int)) < 0) { 
        close(sock); 
        return -1; 
    }

    // V_value2: cada float convertido a orden de red
    for (int i = 0; i < N_value2; i++) {
        uint32_t f_red = float_a_red(V_value2[i]);
        if (send_todo(sock, &f_red, sizeof(uint32_t)) < 0) { close(sock); return -1; }
    }

    int net_x = htonl(value3.x);
    int net_y = htonl(value3.y);
    int net_z = htonl(value3.z);
    if (send_todo(sock, &net_x, sizeof(int)) < 0) { 
        close(sock); 
        return -1; 
    }
    if (send_todo(sock, &net_y, sizeof(int)) < 0) { 
        close(sock); 
        return -1; 
    }
    if (send_todo(sock, &net_z, sizeof(int)) < 0) { 
        close(sock); 
        return -1; 
    }

    // Recibir resultado
    int resultado_red;
    if (recv_todo(sock, &resultado_red, sizeof(int)) < 0) { 
        close(sock); 
        return -1; 
    }

    close(sock);
    return ntohl(resultado_red);
}

int delete_key(char *key) {
    if (key == NULL) return -1;

    int sock = conectar_servidor();
    if (sock < 0) return -1;

    //Opcode: 3 para delete_key
    int op = htonl(3);
    if (send_todo(sock, &op, sizeof(int)) < 0) { 
        close(sock); 
        return -1; 
    }

    char key_buf[256] = {0};
    strncpy(key_buf, key, 255);
    if (send_todo(sock, key_buf, 256) < 0) { 
        close(sock); 
        return -1; 
    }

    int resultado_red;
    if (recv_todo(sock, &resultado_red, sizeof(int)) < 0) {
        close(sock);
        return -1;
    }

    close(sock);
    return ntohl(resultado_red);
}

int exist(char *key) {
    if (key == NULL) return -1;

    int sock = conectar_servidor();
    if (sock < 0) return -1;

    //Opcode: 4 para exist
    int op = htonl(4);
    if (send_todo(sock, &op, sizeof(int)) < 0) { 
        close(sock); 
        return -1; 
    }

    char key_buf[256] = {0};
    strncpy(key_buf, key, 255);
    if (send_todo(sock, key_buf, 256) < 0) { 
        close(sock); 
        return -1; 
    }

    int resultado_red;
    if (recv_todo(sock, &resultado_red, sizeof(int)) < 0) {
        close(sock);
        return -1;
    }

    close(sock);
    return ntohl(resultado_red);
}

int destroy() {
    int sock = conectar_servidor();
    if (sock < 0) return -1;

    //Opcode: 5 para destroy
    int op = htonl(5);
    if (send_todo(sock, &op, sizeof(int)) < 0) {
        close(sock);
        return -1;
    }

    int resultado_red;
    if (recv_todo(sock, &resultado_red, sizeof(int)) < 0) {
        close(sock);
        return -1;
    }

    close(sock);
    return ntohl(resultado_red);
}
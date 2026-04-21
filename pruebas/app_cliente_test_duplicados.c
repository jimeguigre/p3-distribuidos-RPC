#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../claves.h"

int main() {
    char *clave_fija = "CLAVE_REPETIDA";
    char *v1 = "valor prueba";
    float v2[] = {1.0, 2.0};
    struct Paquete v3 = {1, 1, 1};

    printf("Cliente %d: Intentando insertar la clave '%s'...\n", getpid(), clave_fija);

    // Intentamos insertar la misma clave que otros clientes
    int res = set_value(clave_fija, v1, 2, v2, v3);

    if (res == 0) {
        printf("Cliente %d: ¡ÉXITO! He sido el primero en insertar.\n", getpid());
    } else {
        // El servidor debe devolver -1 porque la clave ya existe
        printf("Cliente %d: ERROR esperado (Clave ya existente).\n", getpid());
    }

    return 0;
}
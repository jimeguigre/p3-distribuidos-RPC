//función main que llame a las funciones de la API
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "claves.h"

int main() {
    char v1[256] = "valor inicial 1";
    float v2[] = {1.1, 2.2, 3.3};
    int n2 = 3;
    struct Paquete v3 = {10, 20, 30};

    char res_v1[256];
    float res_v2[32];
    int res_n2;
    struct Paquete res_v3;

    // para las pruebas de concurrencia y para evitar colisiones entre clientes, cada cliente genera una clave única basada en su PID
    char clave_unica[256];
    sprintf(clave_unica, "clave_proceso_%d", getpid());

    printf("--- INICIANDO PLAN DE PRUEBAS ---\n");

    // 1. Probar set_value con la clave única
    if (set_value(clave_unica, v1, n2, v2, v3) == 0) {
        printf("Cliente %d: Inserción de '%s' correcta.\n", getpid(), clave_unica);
    }

    // 2. Probar exist (USANDO CLAVE_UNICA)
    if (exist(clave_unica) == 1) {
        printf("La clave '%s' existe en el sistema.\n", clave_unica);
    }

    // 3. Probar get_value (USANDO CLAVE_UNICA)
    if (get_value(clave_unica, res_v1, &res_n2, res_v2, &res_v3) == 0) {
        printf("Datos recuperados: v1 = '%s', n2 = %d, v3 = {%d,%d,%d}\n", 
                res_v1, res_n2, res_v3.x, res_v3.y, res_v3.z);
    }

    // 4. Probar modify_value (USANDO CLAVE_UNICA)
    char *nuevo_v1 = "valor modificado";
    v3.x = 99;
    if (modify_value(clave_unica, nuevo_v1, n2, v2, v3) == 0) {
        printf("Modificación de '%s' correcta.\n", clave_unica);
    }

    // 5. Verificar modificación (USANDO CLAVE_UNICA)
    if (get_value(clave_unica, res_v1, &res_n2, res_v2, &res_v3) == 0) {
        printf("Datos recuperados tras modificar: v1 = '%s', v3.x = %d\n", res_v1, res_v3.x);
    }

    // 6. Probar delete_key (USANDO CLAVE_UNICA)
    if (delete_key(clave_unica) == 0) {
        printf("Borrado de '%s' correcto.\n", clave_unica);
    }

    // 7. Verificar inexistencia (USANDO CLAVE_UNICA)
    if (exist(clave_unica) == 0) {
        printf("Confirmado: la clave '%s' ya no existe.\n", clave_unica);
    }

    printf("--- PRUEBAS FINALIZADAS ---\n");
    return 0;
}


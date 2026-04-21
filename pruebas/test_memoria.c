#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../claves.h"

#define ITERACIONES 10000

int main() {
    char v1[256] = "datos de prueba para memoria";
    float v2[32] = {1.0, 2.0};
    struct Paquete v3 = {1, 1, 1};
    
    char res_v1[256];
    float res_v2[32];
    int res_n2;
    struct Paquete res_v3;

    printf("--- INICIANDO TEST DE MEMORIA (10,000 peticiones) ---\n");

    // 1. Insertamos una clave base
    if (set_value("mem_key", v1, 2, v2, v3) != 0) {
        // Si ya existe, intentamos modificarla para asegurar que hay datos
        modify_value("mem_key", v1, 2, v2, v3);
    }

    // 2. Bucle de estrés de lectura
    for (int i = 0; i < ITERACIONES; i++) {
        if (get_value("mem_key", res_v1, &res_n2, res_v2, &res_v3) != 0) {
            printf("Error en petición %d\n", i);
            break;
        }
        
        if (i % 1000 == 0) {
            printf("Progreso: %d peticiones completadas...\n", i);
        }
    }

    printf("--- TEST DE MEMORIA FINALIZADO ---\n");
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../claves.h"

int main() {
    // Datos de prueba con valores "reconocibles"
    destroy();
    char *key = "integ_01";
    char *v1 = "Verificando Integridad XDR - 2026";
    int n2 = 5;
    float v2[] = {1.11f, -2.22f, 3.33f, -4.44f, 5.55f};
    struct Paquete v3 = {777, -888, 999};

    printf("--- CLIENTE: ENVIANDO DATOS PARA VERIFICACIÓN ---\n");
    printf("Clave: %s\n", key);
    printf("Value1: %s\n", v1);
    printf("N_Value2: %d\n", n2);
    printf("V_Value2[0]: %.2f, [4]: %.2f\n", v2[0], v2[4]);
    printf("Paquete V3: x=%d, y=%d, z=%d\n", v3.x, v3.y, v3.z);

    if (set_value(key, v1, n2, v2, v3) == 0) {
        printf("--- CLIENTE: Datos enviados con éxito ---\n");
    } else {
        printf("--- CLIENTE: Error en el envío ---\n");
    }

    return 0;
}
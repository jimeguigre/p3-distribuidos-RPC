#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../claves.h"

#define MAX_STR 256
#define MAX_VEC 32

void print_test_result(const char *test_name, int passed) {
    printf("[TEST %s]: %s\n", test_name, passed ? "PASADO" : "FALLADO");
}

int main() {
    destroy();
    printf("--- INICIANDO TEST DE TRANSPARENCIA XDR ---\n");

    // 1. Preparación de datos para "Tipos Complejos"
    char *key = "test_xdr_completo";
    char *v1_original = "Prueba de tipos complejos";
    
    float v2_original[MAX_VEC];
    for (int i = 0; i < MAX_VEC; i++) {
        // Usamos valores con decimales específicos para verificar precisión float
        v2_original[i] = (float)i + 0.12345f;
    }
    
    struct Paquete v3_original = {123456, -987654, 0};

    // 2. Preparación de datos para "Cadenas Límite" (255 caracteres + '\0')
    char key_limite[] = "clave_limite";
    char v1_limite[MAX_STR];
    memset(v1_limite, 'A', 255); // Llenamos 255 posiciones con 'A'
    v1_limite[255] = '\0';        // El carácter 256 es el terminador

    // EJECUCIÓN DE PRUEBAS: 

    // Prueba de inserción de cadena límite
    if (set_value(key_limite, v1_limite, 1, v2_original, v3_original) != 0) {
        fprintf(stderr, "Error al insertar cadena límite\n");
        exit(1);
    }

    // Prueba de inserción de tipos complejos
    if (set_value(key, v1_original, MAX_VEC, v2_original, v3_original) != 0) {
        fprintf(stderr, "Error al insertar tipos complejos\n");
        exit(1);
    }

    // VERIFICACIÓN:
    char res_v1[MAX_STR];
    int res_n2;
    float res_v2[MAX_VEC];
    struct Paquete res_v3;

    // Verificación de Cadena Límite
    get_value(key_limite, res_v1, &res_n2, res_v2, &res_v3);
    int passed_limite = (strlen(res_v1) == 255 && res_v1[254] == 'A');
    print_test_result("Cadenas Límite (255 chars)", passed_limite);

    // Verificación de Tipos Complejos
    get_value(key, res_v1, &res_n2, res_v2, &res_v3);
    
    int passed_v2 = 1;
    for (int i = 0; i < MAX_VEC; i++) {
        if (fabs(res_v2[i] - v2_original[i]) > 0.00001f) {
            passed_v2 = 0;
            break;
        }
    }
    print_test_result("Precisión Vector Float (XDR Array)", passed_v2);

    int passed_v3 = (res_v3.x == v3_original.x && res_v3.y == v3_original.y && res_v3.z == v3_original.z);
    print_test_result("Integridad Estructura Paquete (XDR Struct)", passed_v3);

    printf("--- FINALIZANDO TEST DE TRANSPARENCIA XDR ---\n");
    return 0;
}
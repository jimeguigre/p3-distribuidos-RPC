#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "claves.h"
#include "clavesRPC.h" // para quitar el warning C/C++: Rescan Workspace (ctrl + shift + p) 

/* * Implementación de los procedimientos del servidor.
 * Al usar rpcgen -N -M:
 * 1. Los argumentos se pasan por valor (no punteros a estructuras).
 * 2. El resultado se devuelve por referencia en el segundo parámetro.
 * 3. La función devuelve un bool_t para indicar el éxito de la llamada RPC.
 */

bool_t
destroy_1_svc(int *result, struct svc_req *rqstp)
{
    printf("RPC Servidor: Ejecutando destroy\n");
    *result = destroy(); // Lógica original de claves.c
    return TRUE;
}

bool_t
set_value_1_svc(struct set_value_args args, int *result, struct svc_req *rqstp)
{
    printf("RPC Servidor: Ejecutando set_value para clave: %s\n", args.key);
    
    // Adaptamos los datos a la estructura Paquete de claves.h
    struct Paquete p;
    p.x = args.value3.x;
    p.y = args.value3.y;
    p.z = args.value3.z;

	// LOG DETALLADO PARA PRUEBA DE INTEGRIDAD
    printf("\n--- SERVIDOR: DATOS RECIBIDOS POR RPC ---\n");
    printf("Clave recibida: %s\n", args.key);
    printf("Value1 recibido: %s\n", args.value1);
    printf("N_Value2: %d\n", args.n_value2);
    
    for(int i = 0; i < args.n_value2; i++) {
        printf("V_Value2[%d]: %.2f\n", i, args.v_value2[i]);
    }
    
    printf("Paquete V3: x=%d, y=%d, z=%d\n", 
            args.value3.x, args.value3.y, args.value3.z);
    printf("------------------------------------------\n");

    // Llamamos a la lógica original
    *result = set_value(args.key, args.value1, args.n_value2, args.v_value2, p);
    return TRUE;
}

bool_t
get_value_1_svc(char *key, struct get_value_res *result, struct svc_req *rqstp)
{
    printf("RPC Servidor: Ejecutando get_value para clave: %s\n", key);
    struct Paquete p;

    // Inicializamos la estructura de respuesta para evitar basura en memoria
    memset(result, 0, sizeof(struct get_value_res));

    // Llamamos a la lógica original. Los datos se guardan directamente en la estructura de respuesta RPC.
    //result->resultado = get_value(key, result->value1, &result->n_value2, result->v_value2, &p);
	result->value1 = (char *) malloc(256); // Reservamos memoria para la cadena value1
	result->resultado = get_value(key, result->value1, &result->n_value2, result->v_value2, &p);

    if (result->resultado == 0) {
        result->value3.x = p.x;
        result->value3.y = p.y;
        result->value3.z = p.z;
    }
    return TRUE;
}

bool_t
modify_value_1_svc(struct set_value_args args, int *result, struct svc_req *rqstp)
{
    printf("RPC Servidor: Ejecutando modify_value para clave: %s\n", args.key);
    
    struct Paquete p;
    p.x = args.value3.x;
    p.y = args.value3.y;
    p.z = args.value3.z;

    *result = modify_value(args.key, args.value1, args.n_value2, args.v_value2, p);
    return TRUE;
}

bool_t
delete_key_1_svc(char *key, int *result, struct svc_req *rqstp)
{
    printf("RPC Servidor: Ejecutando delete_key para clave: %s\n", key);
    *result = delete_key(key);
    return TRUE;
}

bool_t
exist_1_svc(char *key, int *result, struct svc_req *rqstp)
{
    printf("RPC Servidor: Ejecutando exist para clave: %s\n", key);
    *result = exist(key);
    return TRUE;
}

/* * Nota: No es necesario implementar el main(). 
 * El archivo generado clavesRPC_svc.c ya contiene el main() que registra el servicio
 * y gestiona el bucle de peticiones[cite: 36, 43].
 */

 /* * Función para liberar memoria del lado del servidor.
 * Es necesaria cuando se compila con el flag -M (Multithread).
 * Se invoca automáticamente después de enviar la respuesta al cliente.
 */
int
claves_prog_1_freeresult (SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
	xdr_free (xdr_result, result);
	return 1;
}
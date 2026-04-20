#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "claves.h"
#include <pthread.h>


#define MAX_HASH 1031 // Número primo para reducir colisiones

typedef struct Nodo {
    char key[256];              // Clave: máx 255 caracteres + '\0'
    char value1[256];           // Valor 1: máx 255 caracteres + '\0'
    int N_value2;               // Dimensión del vector (entre 1 y 32)
    float V_value2[32];         // Vector de floats: máx 32 elementos 
    struct Paquete value3;      // Estructura con enteros x, y, z 
    struct Nodo *siguiente;     // Puntero al siguiente nodo (gestión de colisiones) 
} Nodo;


// La tabla hash global
static Nodo *tabla[MAX_HASH]; 

// Función hash simple utilizando el algoritmo djb2 de Dan Bernstein
unsigned int funcion_hash(char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash % MAX_HASH;
}

// declaración del mutex para proteger el acceso a la tabla hash
pthread_mutex_t cerrojo = PTHREAD_MUTEX_INITIALIZER;


// función set_value: 
int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    // validar N_value2
    if (N_value2 < 1 || N_value2 > 32) return -1;

    pthread_mutex_lock(&cerrojo);
    // calcular índice hash
    unsigned int idx = funcion_hash(key);

    // comprobar si la clave ya existe
    Nodo *actual = tabla[idx];
    while (actual != NULL) {
        if (strcmp(actual->key, key) == 0){
            pthread_mutex_unlock(&cerrojo); // Liberar el cerrojo antes de retornar
            return -1;
        }
        actual = actual->siguiente;
    }

    // crear nuevo nodo 
    Nodo *nuevo = (Nodo *)malloc(sizeof(Nodo));
    if (nuevo == NULL){
        pthread_mutex_unlock(&cerrojo); // Liberar el cerrojo si falla malloc
        return -1;
    }
    
    // copiar datos
    strncpy(nuevo->key, key, 255);
    nuevo->key[255] = '\0';
    strncpy(nuevo->value1, value1, 255);
    nuevo->value1[255] = '\0';
    nuevo->N_value2 = N_value2;
    memcpy(nuevo->V_value2, V_value2, N_value2 * sizeof(float));
    nuevo->value3 = value3;

    // insertar al principio de la lista del índice idx
    nuevo->siguiente = tabla[idx];
    tabla[idx] = nuevo;

    pthread_mutex_unlock(&cerrojo); 
    return 0; // Éxito
}



// función get_value: 
int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {
    pthread_mutex_lock(&cerrojo);
    // calcular índice hash
    unsigned int idx = funcion_hash(key);

    // buscar la clave en la lista enlazada
    Nodo *actual = tabla[idx];
    while (actual != NULL) {
        if (strcmp(actual->key, key) == 0) {            
            // Copiamos la cadena
            strncpy(value1, actual->value1, 255);
            value1[255] = '\0'; // Aseguramos el fin de cadena

            // Devolvemos la dimensión y el vector 
            *N_value2 = actual->N_value2;
            memcpy(V_value2, actual->V_value2, actual->N_value2 * sizeof(float));

            // Devolvemos la estructura Paquete 
            *value3 = actual->value3;

            pthread_mutex_unlock(&cerrojo); 
            return 0; // Éxito 
        }
        actual = actual->siguiente;
    }

    // si llegamos aquí, la clave no existe 
    pthread_mutex_unlock(&cerrojo);
    return -1; 
}


// función modify_value:
int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    // validar N_value2
    if (N_value2 < 1 || N_value2 > 32) return -1;

    pthread_mutex_lock(&cerrojo);
    // calcular índice hash
    unsigned int idx = funcion_hash(key);

    // buscar la clave en la lista enlazada
    Nodo *actual = tabla[idx];
    while (actual != NULL) {
        if (strcmp(actual->key, key) == 0) {
            // actualizar los valores
            strncpy(actual->value1, value1, 255);
            actual->value1[255] = '\0'; // Aseguramos el fin de cadena

            actual->N_value2 = N_value2;
            memcpy(actual->V_value2, V_value2, N_value2 * sizeof(float));
            actual->value3 = value3;

            pthread_mutex_unlock(&cerrojo);
            return 0; // Éxito
        }
        actual = actual->siguiente;
    }

    pthread_mutex_unlock(&cerrojo);

    // si llegamos aquí, la clave no existe
    return -1;
}

// función delete_key:
int delete_key(char *key) {
    pthread_mutex_lock(&cerrojo);
    // calcular índice hash
    unsigned int idx = funcion_hash(key);

    // buscar la clave en la lista enlazada
    Nodo *actual = tabla[idx];
    Nodo *anterior = NULL;
    while (actual != NULL) {
        if (strcmp(actual->key, key) == 0) {
            // eliminar el nodo
            if (anterior == NULL) {
                // el nodo a eliminar es el primero
                tabla[idx] = actual->siguiente;
            } else {
                anterior->siguiente = actual->siguiente;
            }
            free(actual);
            pthread_mutex_unlock(&cerrojo);
            return 0; // Éxito
        }
        anterior = actual;
        actual = actual->siguiente;
    }

    pthread_mutex_unlock(&cerrojo);
    // si llegamos aquí, la clave no existe
    return -1;
}

// función exist:
int exist(char *key) {
    pthread_mutex_lock(&cerrojo);
    // calcular índice hash
    unsigned int idx = funcion_hash(key);

    // buscar la clave en la lista enlazada
    Nodo *actual = tabla[idx];
    while (actual != NULL) {
        if (strcmp(actual->key, key) == 0) {
            pthread_mutex_unlock(&cerrojo);
            return 1; // La clave existe
        }
        actual = actual->siguiente;
    }

    pthread_mutex_unlock(&cerrojo);
    // si llegamos aquí, la clave no existe
    return 0;
}

// función destroy: 
int destroy(void) {
    pthread_mutex_lock(&cerrojo);
    for (int i = 0; i < MAX_HASH; i++) {
        Nodo *actual = tabla[i];
        while (actual != NULL) {
            Nodo *temp = actual;
            actual = actual->siguiente;
            free(temp);
        }
        tabla[i] = NULL; // Limpiar el puntero de la tabla
    }
    pthread_mutex_unlock(&cerrojo);
    return 0; 
}
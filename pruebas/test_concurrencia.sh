#!/bin/bash

# Configuración
export IP_TUPLAS=localhost
NUM_CLIENTES=5

echo "--- INICIANDO TEST DE CONCURRENCIA RPC ---"
echo "Lanzando $NUM_CLIENTES clientes simultáneos..."

# Lanzar clientes en segundo plano
for i in $(seq 1 $NUM_CLIENTES); do
    ./app_cliente > "resultado_cliente_$i.txt" 2>&1 &
    echo "Cliente $i lanzado con éxito (proceso en segundo plano)."
done

# Esperar a que todos los procesos terminen
wait

echo "Todos los clientes han finalizado."
echo "--- VERIFICANDO RESULTADOS ---"

# Comprobar si algún cliente reportó errores
for i in $(seq 1 $NUM_CLIENTES); do
    if grep -q "ERROR" "resultado_cliente_$i.txt"; then
        echo "El cliente $i encontró errores."
    else
        echo "El cliente $i terminó correctamente."
    fi
done

echo "--- TEST DE CONCURRENCIA FINALIZADO ---"
#!/bin/bash
export IP_TUPLAS=localhost
# LD_LIBRARY_PATH es necesario para encontrar libproxyclaves.so [cite: 7, 8]
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH

echo "--- PRUEBA DE EXCLUSIÓN MUTUA RPC (DUPLICADOS) ---"

# Paso previo: Limpiar servidor para que la prueba sea válida
./test_memoria_limpieza 

for i in {1..5}
do
   ./test_duplicados & 
done

wait
echo "--- Prueba finalizada ---"
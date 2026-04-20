CC = gcc
CFLAGS = -Wall -Werror -fPIC

#LDFLAGS: quitamos -lrt porque ya no usamos mqueue
LDFLAGS_LOGICA = -L. -lclaves -lpthread -Wl,-rpath,.
LDFLAGS_PROXY = -L. -lproxyclaves -Wl,-rpath,.
LD_LIBRARY_PATH_EXPORT = export LD_LIBRARY_PATH=.:$$LD_LIBRARY_PATH

all: libclaves.so libproxyclaves.so servidor app_cliente app_cliente_test_duplicados app_cliente_test_carga app_cliente_escritor app_cliente_lector app_cliente_test_protocolo
 
#BIBLIOTECAS
libclaves.so: claves.o
	$(CC) -shared -o libclaves.so claves.o

libproxyclaves.so: proxy-sock.o
	$(CC) -shared -o libproxyclaves.so proxy-sock.o

claves.o: claves.c claves.h
	$(CC) $(CFLAGS) -c claves.c

proxy-sock.o: proxy-sock.c claves.h
	$(CC) $(CFLAGS) -c proxy-sock.c

#EJECUTABLES
servidor: servidor-sock.o libclaves.so
	$(CC) -o servidor servidor-sock.o $(LDFLAGS_LOGICA)

servidor-sock.o: servidor-sock.c claves.h
	$(CC) $(CFLAGS) -c servidor-sock.c

app_cliente: app_cliente.o libproxyclaves.so
	$(CC) -o app_cliente app_cliente.o $(LDFLAGS_PROXY)

app_cliente.o: app_cliente.c claves.h
	$(CC) $(CFLAGS) -c app_cliente.c

#ARCHIVOS DE PRUEBAS
app_cliente_test_duplicados: pruebas/app_cliente_test_duplicados.o libproxyclaves.so
	$(CC) -o app_cliente_test_duplicados pruebas/app_cliente_test_duplicados.o $(LDFLAGS_PROXY)

app_cliente_test_duplicados.o: pruebas/app_cliente_test_duplicados.c claves.h
	$(CC) $(CFLAGS) -c pruebas/app_cliente_test_duplicados.c

app_cliente_test_carga: pruebas/app_cliente_test_carga.o libproxyclaves.so
	$(CC) -o app_cliente_test_carga pruebas/app_cliente_test_carga.o $(LDFLAGS_PROXY)

app_cliente_test_carga.o: pruebas/app_cliente_test_carga.c claves.h
	$(CC) $(CFLAGS) -c pruebas/app_cliente_test_carga.c

app_cliente_escritor: pruebas/app_cliente_escritor.o libproxyclaves.so
	$(CC) -o app_cliente_escritor pruebas/app_cliente_escritor.o $(LDFLAGS_PROXY)

app_cliente_escritor.o: pruebas/app_cliente_escritor.c claves.h
	$(CC) $(CFLAGS) -c pruebas/app_cliente_escritor.c

app_cliente_lector: pruebas/app_cliente_lector.o libproxyclaves.so
	$(CC) -o app_cliente_lector pruebas/app_cliente_lector.o $(LDFLAGS_PROXY)

app_cliente_lector.o: pruebas/app_cliente_lector.c claves.h
	$(CC) $(CFLAGS) -c pruebas/app_cliente_lector.c

app_cliente_test_protocolo: pruebas/app_cliente_test_protocolo.o libproxyclaves.so
	$(CC) -o app_cliente_test_protocolo pruebas/app_cliente_test_protocolo.o $(LDFLAGS_PROXY)

pruebas/app_cliente_test_protocolo.o: pruebas/app_cliente_test_protocolo.c claves.h
	$(CC) $(CFLAGS) -c pruebas/app_cliente_test_protocolo.c -o pruebas/app_cliente_test_protocolo.o

#LIMPIEZA
clean:
	rm -f *.o *.so app_cliente servidor app_cliente_test_duplicados resultado_cliente_*.txt app_cliente_test_carga resultado_carga_*.txt app_cliente_escritor app_cliente_lector app_cliente_test_protocolo
	
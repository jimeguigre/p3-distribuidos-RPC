CC = gcc
CFLAGS = -Wall -fPIC -I/usr/include/tirpc
LDLIBS = -ltirpc

# Banderas para encontrar librerías .so en el directorio actual [cite: 14, 43]
LDFLAGS = -L. -Wl,-rpath,.

# Archivos que rpcgen genera automáticamente [cite: 20]
RPC_GEN = clavesRPC_clnt.c clavesRPC_svc.c clavesRPC_xdr.c clavesRPC.h
RPC_OBJ = clavesRPC_clnt.o clavesRPC_xdr.o clavesRPC_svc.o

all: servidor app_cliente

# 1. Regla única para rpcgen [cite: 20, 45]
# Usamos -C para evitar errores de compatibilidad y generar archivos ANSI C.
$(RPC_GEN): clavesRPC.x
	rpcgen -NM -C clavesRPC.x

# 2. Bibliotecas [cite: 8, 14, 42]
libclaves.so: claves.o
	$(CC) -shared -o libclaves.so claves.o

libproxyclaves.so: proxy-rpc.o clavesRPC_clnt.o clavesRPC_xdr.o
	$(CC) -shared -o libproxyclaves.so proxy-rpc.o clavesRPC_clnt.o clavesRPC_xdr.o $(LDLIBS)

# 3. Compilación de objetos
# Forzamos que los objetos dependan de la generación de los archivos RPC
claves.o: claves.c claves.h
	$(CC) $(CFLAGS) -c claves.c

proxy-rpc.o: proxy-rpc.c clavesRPC.h
	$(CC) $(CFLAGS) -c proxy-rpc.c

clavesRPC_server.o: clavesRPC_server.c clavesRPC.h
	$(CC) $(CFLAGS) -c clavesRPC_server.c

# Regla genérica para los archivos generados por rpcgen
clavesRPC_clnt.o: clavesRPC_clnt.c clavesRPC.h
clavesRPC_svc.o: clavesRPC_svc.c clavesRPC.h
clavesRPC_xdr.o: clavesRPC_xdr.c clavesRPC.h

%.o: %.c
	$(CC) $(CFLAGS) -c $<

# 4. Ejecutables [cite: 11, 16, 43, 49]
servidor: clavesRPC_server.o clavesRPC_svc.o clavesRPC_xdr.o libclaves.so
	$(CC) -o servidor clavesRPC_server.o clavesRPC_svc.o clavesRPC_xdr.o $(LDFLAGS) -lclaves $(LDLIBS) -lpthread

app_cliente: app_cliente.o libproxyclaves.so
	$(CC) -o app_cliente app_cliente.o $(LDFLAGS) -lproxyclaves $(LDLIBS)

clean:
	rm -f *.o *.so servidor app_cliente $(RPC_GEN)
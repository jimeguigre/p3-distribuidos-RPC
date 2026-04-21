struct paquete_rpc {
    int x;
    int y;
    int z;
};

struct set_value_args {
    string key<256>;
    string value1<256>;
    int n_value2;
    float v_value2[32];
    struct paquete_rpc value3;
};

struct get_value_res {
    int resultado;
    string value1<256>;
    int n_value2;
    float v_value2[32];
    struct paquete_rpc value3;
};

program CLAVES_PROG {
    version CLAVES_VERS {
        int DESTROY(void) = 1;
        int SET_VALUE(struct set_value_args) = 2;
        struct get_value_res GET_VALUE(string) = 3;
        int MODIFY_VALUE(struct set_value_args) = 4;
        int DELETE_KEY(string) = 5;
        int EXIST(string) = 6;
    } = 1;
} = 0x20000001;
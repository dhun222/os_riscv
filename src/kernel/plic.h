// Accroding to the PLIC specification documentation by RISCV, 
// it can have 1024 sources and 15872 targets. 
struct plic_source_struct {
    void (*service) (void);
    uint32 prio;
    char name[10];
};

void plic_source_init(void);
void plic_register_source(uint32 id, void *service, uint32 prio, char *name);
void plic_hart_init(int hartid);
uint32 plic_claim(void);
void plic_complete(int intr_id);
void plic_do_service(int intr_id);
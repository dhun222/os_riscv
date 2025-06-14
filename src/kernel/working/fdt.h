#define FDT_BEGIN_NODE  1
#define FDT_END_NODE    2
#define FDT_PROP        3
#define FDT_NOP         4
#define FDT_END         9

struct fdt_header_struct {
    uint32 magic;             
    uint32 totalsize;         
    uint32 off_dt_struct;     
    uint32 off_dt_strings;
    uint32 off_mem_rsvmap;
    uint32 version;
    uint32 last_comp_version;
    uint32 boot_cpuid_phys;
    uint32 size_dt_strings;
    uint32 size_dt_struct;
};

struct fdt_reserved_entry_struct {
    uint64 address;
    uint64 size;
};

uint32 be2le_32(uint32 *addr) {
    char le[4];
    for (int i = 0; i < 4; i++) {
        le[3 - i] = *((char *)addr + i);
    }
    return *((uint32 *)(le));
}

uint64 be2le_64(uint32 *addr) {
    char le[8];
    for (int i = 0; i < 8; i++) {
        le[7 - i] = *((char *)addr + i);
    }
    return *((uint64 *)(le));
}
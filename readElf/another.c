

#include<stdlib.h>
#include<stdio.h>

#include<elf.h>

static inline Elf32_Shdr *elf_sheader(Elf32_Ehdr *hdr) {
	return (Elf32_Shdr *)((int)hdr + hdr->e_shoff);
}

static inline Elf32_Shdr *elf_section(Elf32_Ehdr *hdr, int idx) {
	return &elf_sheader(hdr)[idx];
}

static inline void *elf_load_rel(Elf32_Ehdr *hdr) {
	int result;
	result = elf_load_stage1(hdr);
	if(result == ELF_RELOC_ERR) {
		ERROR("Unable to load ELF file.\n");
		return NULL;
	}
	result = elf_load_stage2(hdr);
	if(result == ELF_RELOC_ERR) {
		ERROR("Unable to load ELF file.\n");
		return NULL;
	}
	// TODO : Parse the program header (if present)
	return (void *)hdr->e_entry;
}
 
void *elf_load_file(void *file) {
	Elf32_Ehdr *hdr = (Elf32_Ehdr *)file;
	if(!elf_check_supported(hdr)) {
		ERROR("ELF File cannot be loaded.\n");
		return;
	}
	switch(hdr->e_type) {
		case ET_EXEC:
			// TODO : Implement
			return NULL;
		case ET_REL:
			return elf_load_rel(hdr);
	}
	return NULL;
}

int main(char* argc[],int argv){
 
    
    return 0;
}
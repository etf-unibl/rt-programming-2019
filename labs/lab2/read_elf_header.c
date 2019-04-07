#include <stdio.h>
#include <stdlib.h>
#include <elf.h>

void read_elfh_32(FILE*);
void read_elfh_64(FILE*);

int main(int argc, char* argv[])
{
	if (argc < 2)
		return -1;
	
	FILE* file = fopen(argv[1], "rb");
	char arc[5];
	
	if(file != NULL)
	{
		printf("ELF Header\n");
		fread(&arc, 1, 5, file);
		fseek(file, 0, 0);
		if(arc[4] == 1)
			read_elfh_32(file);
		else if(arc[4] == 2)
			read_elfh_64(file);
		fclose(file);
	}
	return 0;
}

void read_elfh_32(FILE* file)
{
	Elf32_Ehdr h32;
	fread(&h32, sizeof(h32), 1, file);
	printf("  Magic:   ");
	for(int i = 0; i < EI_NIDENT; ++i)
		printf("%02x ", h32.e_ident[i]);
	printf("\n  Class:                             ");
	switch (h32.e_ident[EI_CLASS]) {
		case ELFCLASSNONE:
			printf("No ELF class");
			break;
		case ELFCLASS32:
			printf("ELF32");
			break;
		case ELFCLASS64:
			printf("ELF64");
			break;
		case ELFCLASSNUM:
			printf("%02x ELF class", h32.e_ident[EI_CLASS]);
			break;
	}
	printf("\n  Data:                              ");
	switch (h32.e_ident[EI_DATA]) {
		case ELFDATANONE:
			printf("No endian convetion");
			break;
		case ELFDATA2LSB:
			printf("Little endian");
			break;
		case ELFDATA2MSB:
			printf("Big endian");
			break;
	}
	printf("\n  Version:                           ");
	switch (h32.e_ident[EI_VERSION]) {
		case EV_NONE:
			printf("0 (no version)");
			break;
		case EV_CURRENT:
			printf("1 (current version)");
			break;
		case EV_NUM:
			printf("%02x version", h32.e_ident[EI_VERSION]);
	}
	printf("\n  OS/ABI:                            ");
	switch (h32.e_ident[EI_OSABI]) {
		case 0x00:
			printf("System V");
			break;
		case 0x01:
			printf("HP-UX");
			break;
		case 0x02:
			printf("NetBSD");
			break;
		case 0x03:
			printf("Linux");
			break;
		case 0x04:
			printf("GNU Hurd");
			break;
		case 0x06:
			printf("Solaris");
			break;
		case 0x07:
			printf("AIX");
			break;
		case 0x08:
			printf("IRIX");
			break;
		case 0x09:
			printf("FreeBSD");
			break;
		case 0x0A:
			printf("Tru64");
			break;
		case 0x0B:
			printf("Novell Modesto");
			break;
		case 0x0C:
			printf("OpenBSD");
			break;
		case 0x0D:
			printf("OpenVMS");
			break;
		case 0x0E:
			printf("NonStop Kernel");
			break;
		case 0x0F:
			printf("AROS");
			break;
		case 0x10:
			printf("Fenix OS");
			break;
		case 0x11:
			printf("CloudABI");
			break;
	}
	printf("\n  ABI Version:                       %x", h32.e_ident[EI_ABIVERSION]);
	printf("\n  Type:                              ");
	switch (h32.e_type) {
		case ET_NONE:
			printf("No type");
			break;
		case ET_REL:
			printf("Release type");
			break;
		case ET_EXEC:
			printf("Executable type");
			break;
		case ET_DYN:
			printf("DYN type");
			break;
		case ET_CORE:
			printf("CORE type");
			break;
		case ET_LOOS:
			printf("LOOS type");
			break;
		case ET_HIOS:
			printf("HIOS type");
			break;
		case ET_LOPROC:
			printf("LOPROC type");
			break;
		case ET_HIPROC:
			printf("HIPROC type");
			break;
	}
	printf("\n  Machine:                           ");
	switch (h32.e_machine) {
		case 0x00:
			printf("No specific instrucion set");
			break;
		case 0x02:
			printf("SPARC");
			break;
		case 0x03:
			printf("x86");
			break;
		case 0x08:
			printf("MIPS");
			break;
		case 0x14:
			printf("PowerPC");
			break;
		case 0x16:
			printf("S390");
			break;
		case 0x28:
			printf("ARM");
			break;
		case 0x2A:
			printf("SuperH");
			break;
		case 0x32:
			printf("IA-64");
			break;
		case 0x3E:
			printf("x86-64");
			break;
		case 0xB7:
			printf("AArch64");
			break;
		case 0xf3:
			printf("RISC-V");
			break;
	}
	printf("\n");
	printf("  Version:                           0x%x\n", h32.e_version);
	printf("  Entry point address:               0x%x\n", h32.e_entry);
	printf("  Start of program headers:          %d (bytes into file)\n", h32.e_phoff);
	printf("  Start of section headers:          %d (bytes into file)\n", h32.e_shoff);
	printf("  Flags:                             0x%x\n", h32.e_flags);
	printf("  Size of this header:               %hu (bytes)\n", h32.e_ehsize);
	printf("  Size of program headers:           %hu (bytes)\n", h32.e_phentsize);
	printf("  Number of program headers:         %hu\n", h32.e_phnum);
	printf("  Size of section headers:           %hu (bytes)\n", h32.e_shentsize);
	printf("  Number of section headers:         %hu\n", h32.e_shnum);
	printf("  Section header string table index: %hu\n", h32.e_shstrndx);
}

void read_elfh_64(FILE* file)
{
	Elf64_Ehdr h64;
	fread(&h64, sizeof(h64), 1, file);
	printf("  Magic:   ");
	for(int i = 0; i < EI_NIDENT; ++i)
		printf("%02x ", h64.e_ident[i]);
	printf("\n  Class:                             ");
	switch (h64.e_ident[EI_CLASS]) {
		case ELFCLASSNONE:
			printf("No ELF class");
			break;
		case ELFCLASS32:
			printf("ELF32");
			break;
		case ELFCLASS64:
			printf("ELF64");
			break;
		case ELFCLASSNUM:
			printf("%02x ELF class", h64.e_ident[EI_CLASS]);
			break;
	}
	printf("\n  Data:                              ");
	switch (h64.e_ident[EI_DATA]) {
		case ELFDATANONE:
			printf("No endian convetion");
			break;
		case ELFDATA2LSB:
			printf("Little endian");
			break;
		case ELFDATA2MSB:
			printf("Big endian");
			break;
	}
	printf("\n  Version:                           ");
	switch (h64.e_ident[EI_VERSION]) {
		case EV_NONE:
			printf("0 (no version)");
			break;
		case EV_CURRENT:
			printf("1 (current version)");
			break;
		case EV_NUM:
			printf("%02x version", h64.e_ident[EI_VERSION]);
	}
	printf("\n  OS/ABI:                            ");
	switch (h64.e_ident[EI_OSABI]) {
		case 0x00:
			printf("System V");
			break;
		case 0x01:
			printf("HP-UX");
			break;
		case 0x02:
			printf("NetBSD");
			break;
		case 0x03:
			printf("Linux");
			break;
		case 0x04:
			printf("GNU Hurd");
			break;
		case 0x06:
			printf("Solaris");
			break;
		case 0x07:
			printf("AIX");
			break;
		case 0x08:
			printf("IRIX");
			break;
		case 0x09:
			printf("FreeBSD");
			break;
		case 0x0A:
			printf("Tru64");
			break;
		case 0x0B:
			printf("Novell Modesto");
			break;
		case 0x0C:
			printf("OpenBSD");
			break;
		case 0x0D:
			printf("OpenVMS");
			break;
		case 0x0E:
			printf("NonStop Kernel");
			break;
		case 0x0F:
			printf("AROS");
			break;
		case 0x10:
			printf("Fenix OS");
			break;
		case 0x11:
			printf("CloudABI");
			break;
	}
	printf("\n  ABI Version:                       %x", h64.e_ident[EI_ABIVERSION]);
	printf("\n  Type:                              ");
	switch (h64.e_type) {
		case ET_NONE:
			printf("No type");
			break;
		case ET_REL:
			printf("Release type");
			break;
		case ET_EXEC:
			printf("Executable type");
			break;
		case ET_DYN:
			printf("DYN type");
			break;
		case ET_CORE:
			printf("CORE type");
			break;
		case ET_LOOS:
			printf("LOOS type");
			break;
		case ET_HIOS:
			printf("HIOS type");
			break;
		case ET_LOPROC:
			printf("LOPROC type");
			break;
		case ET_HIPROC:
			printf("HIPROC type");
			break;
	}
	printf("\n  Machine:                           ");
	switch (h64.e_machine) {
		case 0x00:
			printf("No specific instrucion set");
			break;
		case 0x02:
			printf("SPARC");
			break;
		case 0x03:
			printf("x86");
			break;
		case 0x08:
			printf("MIPS");
			break;
		case 0x14:
			printf("PowerPC");
			break;
		case 0x16:
			printf("S390");
			break;
		case 0x28:
			printf("ARM");
			break;
		case 0x2A:
			printf("SuperH");
			break;
		case 0x32:
			printf("IA-64");
			break;
		case 0x3E:
			printf("x86-64");
			break;
		case 0xB7:
			printf("AArch64");
			break;
		case 0xf3:
			printf("RISC-V");
			break;
	}
	printf("\n");
	printf("  Version:                           0x%x\n", h64.e_version);
	printf("  Entry point address:               0x%lx\n", h64.e_entry);
	printf("  Start of program headers:          %ld (bytes into file)\n", h64.e_phoff);
	printf("  Start of section headers:          %ld (bytes into file)\n", h64.e_shoff);
	printf("  Flags:                             0x%x\n", h64.e_flags);
	printf("  Size of this header:               %hu (bytes)\n", h64.e_ehsize);
	printf("  Size of program headers:           %hu (bytes)\n", h64.e_phentsize);
	printf("  Number of program headers:         %hu\n", h64.e_phnum);
	printf("  Size of section headers:           %hu (bytes)\n", h64.e_shentsize);
	printf("  Number of section headers:         %hu\n", h64.e_shnum);
	printf("  Section header string table index: %hu\n", h64.e_shstrndx);
}

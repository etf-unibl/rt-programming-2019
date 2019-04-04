#include <elf.h>
#include <stdio.h>
void read_elf_header(FILE*,char);

void main(int argc, char **argv)
{
    Elf32_Ehdr header32;
    Elf64_Ehdr header64;
    if(argc<2)
        return;
    char* elfName=argv[1];
    FILE* file= fopen(elfName,"rb");
    char type[5];
    char t;
    if(file)
    {
        fread(&type,5,sizeof(char),file); //reading first 5 bytes of file so I could determin what function to call to write the rest out
        fseek(file,0,0);//returning file pointer to the begining of a file
        if(type[4]==1)
            t=1;
        else
            t=2;
        read_elf_header(file,t);
        fclose(file);
    }
    return;
}

void read_elf_header(FILE* file,char type){
    //Elf64_Ehdr header64;
    //Elf32_Ehdr header32;
    printf("ELF Header \n");
    if(type==1) {
        Elf32_Ehdr header32;
        fread(&header32,1,sizeof(header32),file);
        printf("Magic:          ");
        for(int i=0;i<EI_NIDENT;i++)
            printf("%02x ",header32.e_ident[i]);
        printf("\nClass:                                ");
        switch (header32.e_ident[EI_CLASS]) {
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
                printf("%02x ELF class",header32.e_ident[EI_CLASS]);
                break;
        }
        printf("\nData:                                 ");
        switch (header32.e_ident[EI_DATA]) {
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
        printf("\nVersion:                              ");
        switch (header32.e_ident[EI_VERSION]) {
            case EV_NONE:
                printf("0 (no version)");
                break;
            case EV_CURRENT:
                printf("1 (current version)");
                break;
            case EV_NUM:
                printf("%02x version",header32.e_ident[EI_VERSION]);
        }
        printf("\nOS/ABI                                ");
        switch (header32.e_ident[EI_OSABI]) {
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
        printf("\nABI Version                           %x",header32.e_ident[EI_ABIVERSION]);
        printf("\nType                                  ");
        switch (header32.e_type) {
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
        printf("\nMachine:                              ");
        switch (header32.e_machine) {
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
        printf("\nVersion:                              0x%x",header32.e_version);
        printf("\nEntry point address:                  0x%x",header32.e_entry);
        printf("\nStart of program headers:             %d (bytes into file)",header32.e_phoff);
        printf("\nStart of section headers:             %d (bytes into file)",header32.e_shoff);
        printf("\nFlags:                                0x%x",header32.e_flags);
        printf("\nSize of this header:                  %hu (bytes)",header32.e_ehsize);
        printf("\nSize of program headers:              %hu (bytes)",header32.e_phentsize);
        printf("\nNumber of program headers:            %hu",header32.e_phnum);
        printf("\nSize of section headers:              %hu (bytes)",header32.e_shentsize);
        printf("\nNumber of section headers:            %hu",header32.e_shnum);
        printf("\nSection header string table index:    %hu",header32.e_shstrndx);
        printf("\n");
    }
    else {
        Elf64_Ehdr header64;
        fread(&header64,1,sizeof(header64),file);
        printf("Magic:          ");
        for(int i=0;i<EI_NIDENT;i++)
            printf("%02x ",header64.e_ident[i]);
        printf("\nClass:                                ");
        switch (header64.e_ident[EI_CLASS]) {
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
                printf("%02x ELF class",header64.e_ident[EI_CLASS]);
                break;
        }
        printf("\nData:                                 ");
        switch (header64.e_ident[EI_DATA]) {
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
        printf("\nVersion:                              ");
        switch (header64.e_ident[EI_VERSION]) {
            case EV_NONE:
                printf("0 (no version)");
                break;
            case EV_CURRENT:
                printf("1 (current version)");
                break;
            case EV_NUM:
                printf("%02x version",header64.e_ident[EI_VERSION]);
        }
        printf("\nOS/ABI                                ");
        switch (header64.e_ident[EI_OSABI]) {
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
        printf("\nABI Version                           %x",header64.e_ident[EI_ABIVERSION]);
        printf("\nType                                  ");
        switch (header64.e_type) {
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
        printf("\nMachine:                              ");
        switch (header64.e_machine) {
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
        printf("\nVersion:                              0x%x",header64.e_version);
        printf("\nEntry point address:                  0x%lx",header64.e_entry);
        printf("\nStart of program headers:             %ld (bytes into file)",header64.e_phoff);
        printf("\nStart of section headers:             %ld (bytes into file)",header64.e_shoff);
        printf("\nFlags:                                0x%x",header64.e_flags);
        printf("\nSize of this header:                  %hu (bytes)",header64.e_ehsize);
        printf("\nSize of program headers:              %hu (bytes)",header64.e_phentsize);
        printf("\nNumber of program headers:            %hu",header64.e_phnum);
        printf("\nSize of section headers:              %hu (bytes)",header64.e_shentsize);
        printf("\nNumber of section headers:            %hu",header64.e_shnum);
        printf("\nSection header string table index:    %hu",header64.e_shstrndx);
        printf("\n");
        }
}




















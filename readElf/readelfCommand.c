
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <elf.h>

/* ELF File Header */
//typedef struct
//{
//  unsigned char e_ident[EI_NIDENT];     /* Magic number and other info */
//  Elf32_Half    e_type;                 /* Object file type */
//  Elf32_Half    e_machine;              /* Architecture */
//  Elf32_Word    e_version;              /* Object file version */
//  Elf32_Addr    e_entry;                /* Entry point virtual address */
//  Elf32_Off     e_phoff;                /* Program header table file offset */
//  Elf32_Off     e_shoff;                /* Section header table file offset */
//  Elf32_Word    e_flags;                /* Processor-specific flags */
//  Elf32_Half    e_ehsize;               /* ELF header size in bytes */
//  Elf32_Half    e_phentsize;            /* Program header table entry size */
//  Elf32_Half    e_phnum;                /* Program header table entry count */
//  Elf32_Half    e_shentsize;            /* Section header table entry size */
//  Elf32_Half    e_shnum;                /* Section header table entry count */
//  Elf32_Half    e_shstrndx;             /* Section header string table index */
//} Elf32_Ehdr;

//typedef  struct
//{
//    unsigned char e_ident[16];/*  ELF  identification  */
//    Elf64_Half e_type;/*  Object  file  type  */
//    Elf64_Half e_machine;/*  Machine  type  */
//    Elf64_Word e_version;/*  Object  file  version  */
//    Elf64_Addr e_entry;/*  Entry  point  address  */
//    Elf64_Off e_phoff;/*  Program  header  offset  */
//    Elf64_Off e_shoff;/*  Section  header  offset  */
//    Elf64_Word e_flags;/*  Processor-specific  flags  */
//    Elf64_Half e_ehsize;/*  ELF  header  size  */
//    Elf64_Half e_phentsize;/*  Size  of  program  header  entry  */
//    Elf64_Half e_phnum;/*  Number  of  program  header  entries  */
//    Elf64_Half e_shentsize;/*  Size  of  section  header  entry  */
//    Elf64_Half e_shnum;/*  Number  of  section  header  entries  */
//    Elf64_Half e_shstrndx;/*  Section  name  string  table  index  */    
//} Elf64_Ehdr;


int main(int argv,char *args[]){
     
    //---------------------------------------
    
    if(argv<2) return -1;
    if(strcmp(args[1],"--file-header")!=0) return -1;
    
    printf("\n\n--------------------\n\n");
    
    FILE *file;
    if((file=fopen(args[2],"rb"))==NULL){
     printf("Error occured :)\n");
     return -1;
    }
    
    Elf64_Ehdr *elf=(Elf64_Ehdr*) malloc(sizeof(Elf64_Ehdr));
    fread(elf, 1, sizeof(Elf64_Ehdr), file);
    

   

        printf("File header:\n");
        
        printf("    Magic:  ");
        for(int i=0;i<16;i++)
            printf("%x ",elf->e_ident[i]);
        printf("\n");
        
        printf("    Class:  ");
        printf("%c%c%c",elf->e_ident[1],elf->e_ident[2],elf->e_ident[3]);
        if(elf->e_ident[EI_CLASS]==2)
            printf("64\n");
        else
            printf("32\n");
        
        printf("    Data:  ");
        if(elf->e_ident[EI_DATA]==1)
            printf("2's complement, little endian\n");
        else printf("2's complement, big endian\n");
        
        printf("    Version:  ");
        printf("%d\n",elf->e_ident[EI_VERSION]);
        
        printf("    OS/ABI:  ");
        if(elf->e_ident[EI_OSABI]==0) printf("System V ABI\n");
        if(elf->e_ident[EI_OSABI]==1) printf("HP-UX operating system\n");
        if(elf->e_ident[EI_OSABI]==255) printf("Standalone (embedded) application\n");
        
        
        printf("    ABI Version:  ");
        printf("%d\n",elf->e_ident[EI_ABIVERSION]);
        
        printf("    Type:  ");
        if(elf->e_type==0) printf("No file type\n");
        if(elf->e_type==1) printf("Relocatable object file\n");
        if(elf->e_type==2) printf("Executable file\n");
        if(elf->e_type==3) printf("Shared object file\n");
        if(elf->e_type==4) printf("Core file\n");
        if(elf->e_type==0xfe00) printf("Environment specific use\n");
        if(elf->e_type==0xff00) printf("Processor-specific use\n");
        
        printf("    Machine:  ");
        printf("%d, look it up on http://www.sco.com/developers/gabi/latest/ch4.eheader.html\n", elf->e_machine);
        
        
        printf("    Version:  ");
        printf("%#0x\n",elf->e_ident[EI_VERSION]);
        
        printf("    Entry point address:  ");
        printf("%x\n",elf->e_entry);
        
        printf("    Start of program headers:  ");
        printf("%d (bytes into file)\n",elf->e_phoff);
        
        printf("    Start of section headers:  ");
        printf("%d (bytes into file)\n",elf->e_shoff);
        
        printf("    Flags:  ");
        printf("%#0x\n",elf->e_flags);
        
        printf("    Size of this header:  ");
        printf("%d (bytes)\n",elf->e_ehsize);
        
        printf("    Size of program headers:  ");
        printf("%d (bytes)\n",elf->e_phentsize);
        
        printf("    Number of program headers:  ");
        printf("%d\n",elf->e_phnum);
        
        printf("    Size of section headers:  ");
        printf("%d (bytes)\n",elf->e_shentsize);
        
        printf("    Number of section headers:  ");
        printf("%d\n",elf->e_shnum);
        
        printf("    Section header string table index:  ");
        printf("%d\n",elf->e_shstrndx);
    
    fclose(file);
    printf("\n");
}
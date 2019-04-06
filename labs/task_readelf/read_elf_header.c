/*
 * This program simulates the behavior of readelf -h
 * 
 * Author: Slaven Smiljanic
 *
 * 28/03/2019 22:25 
 *
 */

#include <stdio.h>
#include <elf.h>
#include <stdint.h>

#define ERR_CODE -1
#define SUCCESS 0
#define TRUE 1
#define FALSE (!TRUE)
#define AR32 32
#define AR64 64

// Function declarations:
uint8_t is_elf(Elf64_Ehdr*);
uint8_t read_elf32(const char*);
uint8_t read_elf64(const char*);
uint8_t check_architecture(const char*);

/*
 * File path has to be specified as a command line argument.
 */

uint8_t main(uint8_t argc, char* argv[])
{
    if(1 == argc)
    {
	printf("Specify the ELF file as the argument.\n");
	return ERR_CODE;
    }

    uint8_t architecture = check_architecture(argv[1]);
    if(architecture == AR32)
      read_elf32(argv[1]);

    else if(architecture == AR64)
      read_elf64(argv[1]);
    else
	return ERR_CODE;
    return SUCCESS;
}


/* 
 * This function reads ELF32 header and prints out its content.
 * Parameters:
 *      file_name:  Path to the ELF file
 * Returns:   SUCCESS   (0)  if reading is successful, or
 *            ERR_CODE  (1)  if reading is not successful
 *
*/
uint8_t read_elf32(const char* file_name)
{
    Elf32_Ehdr header;
    FILE* file = fopen(file_name, "rb");
    
    if(file)
    {
	fread(&header, sizeof(header), 1, file);
	if(is_elf(&header))
	{
	
	    printf("ELF Header:\n");

	    // Magic:
	    printf("  Magic:    ");
	    for(uint8_t i = 0; i < EI_NIDENT; i++)
	    {
		printf("%02x ", header.e_ident[i]);
	    }

	    // Class:
	    printf("\n  Class:                             ");
	    switch(header.e_ident[EI_CLASS])
	    {
	    case ELFCLASSNONE:
		printf("Invalid class.");
		break;
	    case ELFCLASS32:
		printf("ELF32");
		break;
	    case ELFCLASS64:
		printf("ELF64");
		break;
	    default:
		break;
	    }

	    // Data:
	    printf("\n  Data:                              ");
	    switch(header.e_ident[EI_DATA])
	    {
	    case ELFDATANONE:
		printf("Invalid data encoding");
		break;
	    case ELFDATA2LSB:
		printf("2's complement, little endian");
		break;
	    case ELFDATA2MSB:
		printf("2's complement, big endian");
		break;
	    default:
		break;
	    }

	    // Version:
	    printf("\n  Version:                           ");
	    switch(header.e_ident[EI_VERSION])
	    {
	    case EV_NONE:
		printf("Invalid version.");
		break;
	    case EV_CURRENT:
		printf("1 (current)");
		break;
	    default:
		break;
	    }

	    // OS/ABI:
	    // This field is often set to 0 regardless of the platform.
	    printf("\n  OS/ABI:                            ");
	    switch(header.e_ident[EI_OSABI])
	    {
	    case 0:
		printf("UNIX - System V");
		break;
	    case 1:
		printf("HP-UX");
		break;
	    case 2:
		printf("NetBSD");
		break;
	    case 3:
		printf("Linux");
		break;
	    case 4:
		printf("GNU Hurd");
		break;
	    case 6:
		printf("Solaris");
		break;
	    case 7:
		printf("AIX");
		break;
	    case 8:
		printf("IRIX");
		break;
	    case 9:
		printf("FreeBSD");
		break;
	    case 10:
		printf("Tru64");
		break;
	    case 11:
		printf("Novell Modesto");
		break;
	    case 12:
		printf("OpenBSD");
		break;
	    case 13:
		printf("OpenVMS");
		break;
	    case 14:
		printf("NonStop Kernel");
		break;
	    case 15:
		printf("AROS");
		break;
	    case 16:
		printf("Fenix OS");
		break;
	    case 17:
		printf("CloudABI");
		break;
	    default:
		break;
	    }

	    // ABI Version:
	    // Linux has no definition of this - will be 0.
	    printf("\n  ABI Version:                       ");
	    printf("%d", header.e_ident[EI_ABIVERSION]);

	    // Type:
	    printf("\n  Type:                              ");
	    switch(header.e_type)
	    {
	    case ET_NONE:
		printf("NONE (No file type)");
		break;
	    case ET_REL:
		printf("REL (Relocatable file)");
		break;
	    case ET_EXEC:
		printf("EXEC (Executable file)");
		break;
	    case ET_DYN:
		printf("DYN (Shared object file)");
		break;
	    case ET_CORE:
		printf("CORE (Core file)");
		break;
	    case ET_LOPROC:
		printf("LOPROC (Processor-specific)");
		break;
	    case ET_HIPROC:
		printf("HIPROC (Processor-specific)");
		break;
	    default:
		break;
	    }

	    // Machine:
	    printf("\n  Machine:                           ");
	    switch(header.e_machine)
	    {
	    case EM_NONE:
		printf("No machine");
		break;
	    case 0x28:
		printf("ARM");
		break;
	    case 0x03:
		printf("x86");
		break;
	    case 0x32:
		printf("IA-64");
		break;
	    case 0x3E:
		printf("Advanced Micro Devices X86-64");
		break;
	    default:
		break;
	    }

	    // Version:
	    // This field is set to 1 for the original version of ELF.
	    printf("\n  Version:                           ");
	    printf("%#x", header.e_version);

	    // Entry point address:
	    printf("\n  Entry point address:               ");
	    printf("%#lx", header.e_entry);

	    // Start of program headers:
	    printf("\n  Start of program headers:          ");
	    printf("%ld (bytes into file)", header.e_phoff);

	    // Start of section headers:
	    printf("\n  Start of section headers:          ");
	    printf("%ld (bytes into file)", header.e_shoff);

	    // Flags:
	    printf("\n  Flags:                             ");
	    printf("%#x", header.e_flags);

	    // Size of this header:
	    printf("\n  Size of this header:               ");
	    printf("%d (bytes)", header.e_ehsize);

	    // Size of program headers:
	    printf("\n  Size of program headers:           ");
	    printf("%d (bytes)", header.e_phentsize);

	    // Number of program headers:
	    printf("\n  Number of program headers:         ");
	    printf("%d", header.e_phnum);

	    // Size of section headers:
	    printf("\n  Size of section headers:           ");
	    printf("%d (bytes)", header.e_shentsize);

	    // Number of section headers:
	    printf("\n  Number of section headers:         ");
	    printf("%d", header.e_shnum);

	    // Section header string table index:
	    printf("\n  Section header string table index: ");
	    printf("%d\n", header.e_shstrndx);
	    
	}
	else
	{
	    // This happens when Magic doesn't correspond to ELF format.
	    printf("Not an ELF file.\n");
	    return ERR_CODE;
	}
    }
    else
    {
	// This happens if there is no file on a specified path.
	// Java equivalent: FileNotFoundException: (No such file or directory)
	printf("Error while opening the file.\n");
	return ERR_CODE;
    }

    return SUCCESS;
}


/* 
 * This function reads ELF64 header and prints out its content.
 * Parameters:
 *      file_name:  Path to the ELF file
 * Returns:   SUCCESS   (0)  if reading is successful, or
 *            ERR_CODE  (1)  if reading is not successful
 *
*/
uint8_t read_elf64(const char* file_name)
{
    Elf64_Ehdr header;
    FILE* file = fopen(file_name, "rb");
    
    if(file)
    {
	fread(&header, sizeof(header), 1, file);
	if(is_elf(&header))
	{
	
	    printf("ELF Header:\n");

	    // Magic:
	    printf("  Magic:    ");
	    for(uint8_t i = 0; i < EI_NIDENT; i++)
	    {
		printf("%02x ", header.e_ident[i]);
	    }

	    // Class:
	    printf("\n  Class:                             ");
	    switch(header.e_ident[EI_CLASS])
	    {
	    case ELFCLASSNONE:
		printf("Invalid class.");
		break;
	    case ELFCLASS32:
		printf("ELF32");
		break;
	    case ELFCLASS64:
		printf("ELF64");
		break;
	    default:
		break;
	    }

	    // Data:
	    printf("\n  Data:                              ");
	    switch(header.e_ident[EI_DATA])
	    {
	    case ELFDATANONE:
		printf("Invalid data encoding");
		break;
	    case ELFDATA2LSB:
		printf("2's complement, little endian");
		break;
	    case ELFDATA2MSB:
		printf("2's complement, big endian");
		break;
	    default:
		break;
	    }

	    // Version:
	    printf("\n  Version:                           ");
	    switch(header.e_ident[EI_VERSION])
	    {
	    case EV_NONE:
		printf("Invalid version.");
		break;
	    case EV_CURRENT:
		printf("1 (current)");
		break;
	    default:
		break;
	    }

	    // OS/ABI:
	    // This field is often set to 0 regardless of the platform.
	    printf("\n  OS/ABI:                            ");
	    switch(header.e_ident[EI_OSABI])
	    {
	    case 0:
		printf("UNIX - System V");
		break;
	    case 1:
		printf("HP-UX");
		break;
	    case 2:
		printf("NetBSD");
		break;
	    case 3:
		printf("Linux");
		break;
	    case 4:
		printf("GNU Hurd");
		break;
	    case 6:
		printf("Solaris");
		break;
	    case 7:
		printf("AIX");
		break;
	    case 8:
		printf("IRIX");
		break;
	    case 9:
		printf("FreeBSD");
		break;
	    case 10:
		printf("Tru64");
		break;
	    case 11:
		printf("Novell Modesto");
		break;
	    case 12:
		printf("OpenBSD");
		break;
	    case 13:
		printf("OpenVMS");
		break;
	    case 14:
		printf("NonStop Kernel");
		break;
	    case 15:
		printf("AROS");
		break;
	    case 16:
		printf("Fenix OS");
		break;
	    case 17:
		printf("CloudABI");
		break;
	    default:
		break;
	    }

	    // ABI Version:
	    // Linux has no definition of this - will be 0.
	    printf("\n  ABI Version:                       ");
	    printf("%d", header.e_ident[EI_ABIVERSION]);

	    // Type:
	    printf("\n  Type:                              ");
	    switch(header.e_type)
	    {
	    case ET_NONE:
		printf("NONE (No file type)");
		break;
	    case ET_REL:
		printf("REL (Relocatable file)");
		break;
	    case ET_EXEC:
		printf("EXEC (Executable file)");
		break;
	    case ET_DYN:
		printf("DYN (Shared object file)");
		break;
	    case ET_CORE:
		printf("CORE (Core file)");
		break;
	    case ET_LOPROC:
		printf("LOPROC (Processor-specific)");
		break;
	    case ET_HIPROC:
		printf("HIPROC (Processor-specific)");
		break;
	    default:
		break;
	    }

	    // Machine:
	    printf("\n  Machine:                           ");
	    switch(header.e_machine)
	    {
	    case EM_NONE:
		printf("No machine");
		break;
	    case 0x28:
		printf("ARM");
		break;
	    case 0x03:
		printf("x86");
		break;
	    case 0x32:
		printf("IA-64");
		break;
	    case 0x3E:
		printf("Advanced Micro Devices X86-64");
		break;
	    default:
		break;
	    }

	    // Version:
	    // This field is set to 1 for the original version of ELF.
	    printf("\n  Version:                           ");
	    printf("%#x", header.e_version);

	    // Entry point address:
	    printf("\n  Entry point address:               ");
	    printf("%#lx", header.e_entry);

	    // Start of program headers:
	    printf("\n  Start of program headers:          ");
	    printf("%ld (bytes into file)", header.e_phoff);

	    // Start of section headers:
	    printf("\n  Start of section headers:          ");
	    printf("%ld (bytes into file)", header.e_shoff);

	    // Flags:
	    printf("\n  Flags:                             ");
	    printf("%#x", header.e_flags);

	    // Size of this header:
	    printf("\n  Size of this header:               ");
	    printf("%d (bytes)", header.e_ehsize);

	    // Size of program headers:
	    printf("\n  Size of program headers:           ");
	    printf("%d (bytes)", header.e_phentsize);

	    // Number of program headers:
	    printf("\n  Number of program headers:         ");
	    printf("%d", header.e_phnum);

	    // Size of section headers:
	    printf("\n  Size of section headers:           ");
	    printf("%d (bytes)", header.e_shentsize);

	    // Number of section headers:
	    printf("\n  Number of section headers:         ");
	    printf("%d", header.e_shnum);

	    // Section header string table index:
	    printf("\n  Section header string table index: ");
	    printf("%d\n", header.e_shstrndx);
	    
	}
	else
	{
	    // This happens when Magic doesn't correspond to ELF format.
	    printf("Not an ELF file.\n");
	    return ERR_CODE;
	}
    }
    else
    {
	// This happens if there is no file on a specified path.
	// Java equivalent: FileNotFoundException: (No such file or directory)
	printf("Error while opening the file.\n");
	return ERR_CODE;
    }

    return SUCCESS;
}

/*
 * This function checks if the given file is in ELF format.
 * Parameters: 
 *      header:  Elf64_Ehdr type file header (type defined in elf.h)
 * Returns:   TRUE   (1)   if the file is in the ELF format, or
 *            FALSE  (0)   if the file is not in the ELF format
 */
uint8_t is_elf(Elf64_Ehdr* header)
{
    if(header->e_ident[1] == 0x45 &&
       header->e_ident[2] == 0x4c &&
       header->e_ident[3] == 0x46)
	return TRUE;
    else
	return FALSE;
}


/*
 * This function checks if the file is 64 or 32 bit.
 * Parameters:
 *      file_name:  string that represents a path to file.
 * Returns:  32, 64 or ERR_CODE
 *      
 */
uint8_t check_architecture(const char* file_name)
{
    FILE* file = fopen(file_name, "rb");
    Elf64_Ehdr header;
    if(file)
    {
	fread(&header, sizeof(header), 1, file);
	if(header.e_ident[EI_CLASS] == ELFCLASS32) return AR32;
	else if(header.e_ident[EI_CLASS] == ELFCLASS64) return AR64;
	else
	    return ERR_CODE;
    }
}


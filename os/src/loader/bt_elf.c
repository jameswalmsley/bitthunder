/**
 *	ELF decoder library.
 *
 **/

#include <bitthunder.h>
#include <loader/bt_loader.h>
#include <loader/elf.h>
#include <string.h>

static BT_BOOL bt_elf_hdr_valid(Elf32_Ehdr *hdr) {
	BT_u8 *h = (BT_u8 *) hdr;
	if(h[EI_MAG0] == ELFMAG0 &&
	   h[EI_MAG1] == ELFMAG1 &&
	   h[EI_MAG2] == ELFMAG2 &&
	   h[EI_MAG3] == ELFMAG3) {
		return BT_TRUE;
	}
	return BT_FALSE;
}

static Elf32_Sym *bt_elf_find_sym(const BT_i8 *name, Elf32_Shdr *shdr, const BT_i8 *strings, void *elf_start) {
	Elf32_Sym *syms = (Elf32_Sym *) (elf_start + shdr->sh_offset);
	BT_u32 i;

	for(i = 0; i < shdr->sh_size / sizeof(Elf32_Sym); i++) {
		if(!strcmp(name, strings + syms[i].st_name)) {
			return &syms[i];
		}
	}

	return NULL;
}

static BT_BOOL bt_elf_can_decode(void *image_start, BT_u32 len) {
	Elf32_Ehdr *hdr = (Elf32_Ehdr *) image_start;
	return bt_elf_hdr_valid(hdr);
}

void *bt_elf_image_load(void *elf_start, BT_u32 len, BT_LOADER_SECTION_CB pfnLoad, BT_ERROR *pError) {
	Elf32_Ehdr	*hdr 	= NULL;
	Elf32_Phdr 	*phdr 	= NULL;
	Elf32_Shdr  *shdr 	= NULL;
	void 		*entry 	= NULL;
	BT_u32 		 i;
	BT_i8 		*strings = NULL;

	hdr = (Elf32_Ehdr *) elf_start;
	if(!bt_elf_hdr_valid(hdr)) {
		BT_kPrint("elfload: invalid ELF image");
		return NULL;
	}


	/*
	 *	First iterate the program headers to load text and data sections.
	 */
	phdr = (Elf32_Phdr *) elf_start + hdr->e_phoff;
	for(i = 0; i < hdr->e_phnum; ++i) {
		switch(phdr[i].p_type) {
		case PT_NULL:
			break;

		case PT_LOAD: {
			BT_u32 start = phdr[i].p_vaddr;
			BT_u32 end = (phdr[i].p_vaddr + phdr[i].p_filesz) - 1;
			BT_u32 size = phdr[i].p_filesz;

			BT_kPrint("elfload: load : %08x - %08x (%d)", start, end, size);
			pfnLoad((void *) start, (elf_start + phdr[i].p_offset), phdr[i].p_flags, size);
			break;
		}

		case PT_DYNAMIC:
		case PT_NOTE:
		case PT_INTERP:
		case PT_PHDR:
			BT_kPrint("elfload: unsupported header type");
			break;
		default:
			BT_kPrint("elfload: invalid header");
			break;
		}
	}

	/*
	 *	Iterate the section headers to get symbol tables etc.
	 */
	shdr = (Elf32_Shdr *) (elf_start + hdr->e_shoff);
	for(i = 0; i < hdr->e_shnum; ++i) {
		switch(shdr[i].sh_type) {
		case SHT_NULL:
		case SHT_PROGBITS:
		case SHT_STRTAB:
		case SHT_HASH:
		case SHT_DYNAMIC:
		case SHT_MIPS_GPTAB:
		case SHT_REL:
		case SHT_DYNSYM:
			break;

		case SHT_SYMTAB: {
			strings = elf_start + shdr[shdr[i].sh_link].sh_offset;
			Elf32_Sym *entry_sym = bt_elf_find_sym("_start", shdr + i, strings, elf_start);
			if(entry_sym) {
				entry = (void *) entry_sym->st_value;
			}
			break;
		}

		default:
			break;
		}
	}

	BT_kPrint("elfload: entry (_start) = %08x", entry);

	return entry;
}

static void *bt_elf_decode(void *image_start, BT_u32 len, BT_LOADER_SECTION_CB pfnLoad, BT_ERROR *pError) {
	return bt_elf_image_load(image_start, len, pfnLoad, pError);
}

BT_LOADER_DEF oElfLoader = {
	.name = "elf",
	.pfnCanDecode = bt_elf_can_decode,
	.pfnDecode = bt_elf_decode,
};

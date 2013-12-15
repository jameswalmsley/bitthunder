#ifndef _BT_LOADER_H_
#define _BT_LOADER_H_

typedef struct _BT_LOADER_SEGMENT {
	void   *v_addr;		// Virtual address where section should appear.
	void   *data;		// Pointer to data/code to be copied into the section.
	BT_u32 	data_len;	// Length of data to be copied.
	BT_u32 	size;		// Section length in memory. >= len.
	BT_u32	flags;		// Memory segment flags.
} BT_LOADER_SEGMENT;


typedef void (*BT_LOADER_SEGMENT_CB) (BT_LOADER_SEGMENT *pSegment, void *pParam);

typedef struct _BT_LOADER {
	const BT_i8 *name;
	BT_BOOL (*pfnCanDecode)(void *image_start, BT_u32 len);
	void   *(*pfnDecode)(void *image_start, BT_u32 len, BT_LOADER_SEGMENT_CB pfnLoad, void *pParam, BT_ERROR *pError);
} BT_LOADER;

#define BT_LOADER_DEF	static const BT_ATTRIBUTE_SECTION(".bt.loaders") BT_LOADER

BT_ERROR BT_ExecImageFile(const BT_i8 *szpPath);

#endif

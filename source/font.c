#include "exe3.h"

#define _8x16_COUNTER 40
#define _16x16_COUNTER 50

#define VRAMFONTBUFFEROFFSET_16x16 (0x6005800 + 11 * 0x20 * 2 + _8x16_COUNTER * 0x20 * 2)
#define VRAMFONTBUFFEROFFSET_8x16 (0x6005800)
#define FONTINROMOFFSET 0x8800000
#define CODELIST_16x16 (CodeItem*)(0x6005800 + 11 * 0x20 * 2 + _8x16_COUNTER * 0x20 * 2 + _16x16_COUNTER * 4 * 0x20 + 4 * _8x16_COUNTER)
#define CODELIST_8x16 (CodeItem*)(0x6005800 + 11 * 0x20 * 2 + _8x16_COUNTER * 0x20 * 2 + _16x16_COUNTER * 4 * 0x20)
#define PALETTE_OFFSET 0x2009910
#define ENABLE_LAYER_FONT (*(u32 *)PALETTE_OFFSET == 0x7FFE56B5)

//为了确保vram的访问正常，使用位字段
typedef struct _codeItem
{
	u32 code:16;
	u32 layer:8;
	u32 index:8;
} CodeItem;

typedef struct _chipEntry
{
	u16 index;
	u16 code;
} ChipEntry;

typedef struct _chipEntry2
{
	u8 unk0[10];
	u16 code;
	u8 unk1[16];
	u32 index;
} ChipEntry2;

typedef struct _glyphTable
{
	CodeItem * codeList;
	int counter;
} GlyphTable;

GlyphTable glyphTable_8x16 = {CODELIST_8x16, 40 };
GlyphTable glyphTable_16x16 = {CODELIST_16x16, _16x16_COUNTER * 2};

void InitCodeList()
{
	CodeItem * codeList = glyphTable_8x16.codeList;
	volatile CodeItem item;
	item.code = 0xffff;
	item.layer = 0;
	item.index = 11;
	for(int i = 0;i < glyphTable_8x16.counter;i++)
	{
		*(codeList + i) = item;
		item.index++;
	}
	
	item.index = 0;
	codeList = glyphTable_16x16.codeList;
	int i = 0;
	for(;i < glyphTable_16x16.counter / 2;i++)
	{
		*(codeList + i) = item;
		item.index++;
	}
	
	item.layer = 1;
	item.index = 0;
	for(;i < glyphTable_16x16.counter;i++)
	{
		*(codeList + i) = item;
		item.index++;
	}
}

//使用数组结构，效率的瓶颈，如果是链表就没这个问题，可惜内存有限
static inline void MoveCodeItemToStart(GlyphTable * glyphTable, int index)
{
	CodeItem * codeList = glyphTable->codeList;
	if(index >= glyphTable->counter) return;
	volatile CodeItem lastcode = codeList[index];
	for(int i = index - 1;i >= 0;i--)
	{
		codeList[i + 1] = codeList[i];
	}
	*codeList = lastcode;
}

static inline int SearchCode(GlyphTable * glyphTable,u32 code,int * lastUnusedIndex)
{
	CodeItem * codeList = glyphTable->codeList;
	int i;
	*lastUnusedIndex = -1;
	int counter = glyphTable->counter;
	if (!ENABLE_LAYER_FONT)
	{
		if (counter > _16x16_COUNTER) counter = _16x16_COUNTER;
	}
	for(i = 0;i < counter;i++)
	{
		if(codeList[i].code == code)
		{
			return i;
		}
		else if(codeList[i].code == 0xffff)
		{
			*lastUnusedIndex = i;
			return -1;
		}
	}
	*lastUnusedIndex = counter - 1;//使用索引表最后一个
	return -1;
}

static inline int getCodeIndex(u8 * str)
{
	u8 h = *str;
	if (h < 0x80)
		return h;
	else
	{
		u8 l = *(str + 1);
		return ((((h - 0x80) << 7) + l) << 1) + 0x80;
	}
}

void CopyGlyph_8x16(int indexInVramFont, int layer, int code)
{
	u8 * temp = &code;
	int indexInFont = getCodeIndex(temp);//计算原始字库中的偏移
	if(indexInFont == -1) return;
	u32 * src = (u32*)(FONTINROMOFFSET + indexInFont * 0x40);
	u32 * dest = (u32*)(VRAMFONTBUFFEROFFSET_8x16 + indexInVramFont * 0x40);
	for(int i = 0;i < 0x10;i++)
		*dest++ = *src++;//复制字模
}


void CopyGlyph_16x16(int indexInVramFont, int layer, int code)
{
	u32 mask = layer ? 0x33333333 : 0xcccccccc;
	u8 * temp = &code;
	int indexInFont = getCodeIndex(temp);//计算原始字库中的偏移
	if(indexInFont == -1) return;
	u32 * src = (u32*)(FONTINROMOFFSET + indexInFont * 0x40);
	u32 * dest = (u32*)(VRAMFONTBUFFEROFFSET_16x16 + indexInVramFont * 0x80);
	for(int i = 0;i < 0x10 * 2;i++)
	{
		*dest = (*dest & mask) | (*src << (layer << 1));//
		dest++;
		src++;
	}
}

int GetIndex(GlyphTable * glyphTable,u32 code, void (*CopyGlyph)(int, int, int), int * layer)
{
	CodeItem * codeList = glyphTable->codeList;
	int lastUnused;
	int index = SearchCode(glyphTable,code,&lastUnused);
	if(index == -1)
	{
		volatile CodeItem item = codeList[lastUnused];
		item.code = code;
		codeList[lastUnused] = item;
		MoveCodeItemToStart(glyphTable,lastUnused);//移动到最前面，LRU缓存算法的核心
		*layer = item.layer;
		CopyGlyph(item.index, item.layer, code);
		return item.index;
	}
	else
	{
		int glyphindex = codeList[index].index;
		*layer = codeList[index].layer;
		MoveCodeItemToStart(glyphTable,index);
		return glyphindex;//获取字库在显存中的偏移
	}
}

static inline int GetCode(u8 * str,int * length)
{
	u8 h = *str;
	if (h < 0x80)
	{
		*length = 1;
		return h;
	}
	else
	{
		*length = 2;
		u8 l = *(str + 1);
		return (l << 8) | h;
	}
}

int procMapGen(u8 * str,u16 * mapUP,u16 * mapDown,u16 mapdata, u32 * length)
{
	int i = 0;
	int layer = 0;
	for(; *str != 0xE7; )
	{
		int len;
		int code;
		restart:
		code = GetCode(str,&len);
		str += len;
		int index;
		if (code < 11)
		{
			*mapUP++ = mapdata + (code * 2);
			*mapDown++ = mapdata + (code * 2 + 1);
			i++;
		}
		else if (code < 0x80)
		{
			index = GetIndex(&glyphTable_8x16,code, CopyGlyph_8x16, &layer);
			*mapUP++ = mapdata + (index * 2);
			*mapDown++ = mapdata + (index * 2 + 1);
			i++;
		}
		else
		{
			index = GetIndex(&glyphTable_16x16,code, CopyGlyph_16x16, &layer);
			index = index * 4  + (11 + glyphTable_8x16.counter) * 2;
			u16 mapdata_t = mapdata;
			if (ENABLE_LAYER_FONT)
			{
				mapdata_t = (mapdata_t & 0xFFF) | ((8 + layer) << 12);
			}
			*mapUP++ = mapdata_t + index;
			*mapUP++ = mapdata_t + (index + 2);
			*mapDown++ = mapdata_t + (index + 1);
			*mapDown++ = mapdata_t + (index + 3);
			i+=2;
		}
	}
	*length = i;
	for (;i < 8;i++)
	{
			*mapUP++ = 0;
			*mapDown++ = 0;
	}
	int result = i;
	return result;
}

void CopyPalette()
{
	static u16 palette[] = {0x56B5, 0x7FFE, 0x2D6B, 0x739C, 0x56B5, 0x7FFE, 0x2D6B, 0x739C, 0x56B5, 0x7FFE, 0x2D6B, 0x739C,0x56B5 , 0x7FFE, 0x2D6B, 0x739C,
						0x56B5, 0x56B5, 0x56B5, 0x56B5, 0x7FFE, 0x7FFE, 0x7FFE, 0x7FFE, 0x2D6B, 0x2D6B, 0x2D6B, 0x2D6B, 0x739C, 0x739C, 0x739C, 0x739C};
	for(int i = 0; i < sizeof(palette); i++)
		*((u16*)PALETTE_OFFSET + i) = palette[i];
}

void hook_sub3007474(u32 * regs)
{
	regs[0] = procMapGen(regs[0],regs[2],regs[3],regs[6], &regs[7]);
	regs[2] += 16;
	regs[3] += 16;
}

void hook_sub8035fc8(u32 * regs)
{
	CopyPalette();
	callOrignalFunc(regs, 0x8035fc9);
}

void hook_sub8034058(u32 * regs)
{
	CopyPalette();
	callOrignalFunc(regs, 0x8034059);
}

void hook_sub803362C(u32 chipOffset, ChipEntry * chipList, u16 * map)
{
	u16 mapdata = 0x42C0;
	int layer = 0;
	for(int i = 0; i < 7; i++)
	{
		u16 chipIndex = chipList[chipOffset + i].index;
		if (chipIndex == 0)
		{
			*map++ = 0;
			*map++ = 0;
			continue;
 		}
		u16 code = chipList[chipOffset + i].code;
		if (code == 0x1a) code+=0x21;
		int index = GetIndex(&glyphTable_8x16, code + 11, CopyGlyph_8x16, &layer);
		*map++ = mapdata + (index * 2);
		*map++ = mapdata + (index * 2 + 1);
	}
}

void hook_sub8033BF0(u32 chipOffset, ChipEntry2 * chipList, u16 * map)
{
	u16 mapdata = 0x42C0;
	int layer = 0;
	for(int i = 0; i < 7; i++)
	{
		u16 chipIndex = chipList[chipOffset + i].index;
		if (chipIndex == 0)
		{
			*map++ = 0;
			*map++ = 0;
			continue;
 		}
		u16 code = chipList[chipOffset + i].code;
		if (code == 0x1a) code+=0x21;
		int index = GetIndex(&glyphTable_8x16, code + 11, CopyGlyph_8x16, &layer);
		*map++ = mapdata + (index * 2);
		*map++ = mapdata + (index * 2 + 1);
	}
}

u32* copyRegs(u32 * dest,u32 * src)
{
	int i = 0;
	for (; i < 8; i++)
		dest[i] = src[i + 5];
	for (; i < 13; i++)
		dest[i] = src[i - 8];
	return dest;
}

u32* restoreRegs(u32 * dest,u32 * src)
{
	int i = 0;
	for (; i < 5; i++)
		dest[i] = src[i + 8];
	for (; i < 13; i++)
		dest[i] = src[i - 5];
	return dest;
}
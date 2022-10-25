#include <string.h>

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef volatile unsigned char   vu8;
typedef volatile unsigned short  vu16;
typedef volatile unsigned int    vu32;

#include "font_8x8.h"

typedef u32 Tile[16];
typedef Tile   TileBlock[256];

#define MEM_IO		0x04000000	//!< I/O registers

#define REG_TM0D			*(vu16*)(MEM_IO+0x0100)	//!< Timer 0 data
#define REG_TM0CNT			*(vu16*)(MEM_IO+0x0102)	//!< Timer 0 control
#define REG_TM1D			*(vu16*)(MEM_IO+0x0104)	//!< Timer 1 data
#define REG_TM1CNT			*(vu16*)(MEM_IO+0x0106)	//!< Timer 1 control
#define REG_TM2D			*(vu16*)(MEM_IO+0x0108)	//!< Timer 2 data
#define REG_TM2CNT			*(vu16*)(MEM_IO+0x010A)	//!< Timer 2 control
#define REG_TM3D			*(vu16*)(MEM_IO+0x010C)	//!< Timer 3 data
#define REG_TM3CNT			*(vu16*)(MEM_IO+0x010E)	//!< Timer 3 control

#define MODOVIDEO_0    0x0000
#define ENABLE_OBJECTS 0x1000
#define MAPPINGMODE_1D 0x0040
#define MODOVIDEO_3 0x0003
#define MODOVIDEO_4 0x0004
#define BGENABLE2 0x0400

#define REGISTRO_DISPLAY *((u32*)0x04000000)

#define MEM_TILE      ((TileBlock*)0x6000000 )
#define MEM_PALETTE   ((u16*)(0x05000200))

#define  BG_PALETTE_MEM  ((u16 *) 0x5000000)
#define BACK_BUFFER     0x10
#define BUFFERPANTALLA ((u16*)0x06000000)
#define  VIDEO_BUFFER_0  ((u16 *) 0x6000000)
#define  VIDEO_BUFFER_1  ((u16 *) 0x600A000)
#define PANTALLA_W 240
#define PANTALLA_H 160
#define REG_VCOUNT (*(volatile u16*)0x04000006)

#define MEM_OAM ((volatile ObjectAttributes *)0x07000000)

u16 *current_video_buffer = VIDEO_BUFFER_0;
u32 background_color = 0x0;

u32 cursor = 0;

void put_character(char c, int x, int y, u16 *video_buffer, int erase) {
	int font_y = (((int) c) - 32) / 16;
	int font_x = (((int) c) - 32) % 16;
	int i, j;
	int font_offset = (font_y * 64 * 8) + (font_x * 4);
	int offset = ((y * 240) + x) >> 1;
	for (i = 0; i < 8; i++, offset += 120, font_offset += 64) {
		if (erase) {
			for (j = 0; j < 4; j++)
				video_buffer[offset + j] = 0x0000;
		}
		for (j = 0; j < 4; j++) {
			register u16 d = fontData[font_offset + j];
			register u16 prev = video_buffer[offset + j];
			register u16 to_write = 0;
			if ((d & 0x00FF) != fontTransparentIndex)
				to_write |= (d & 0x00FF);
			else
				to_write |= (prev & 0x00FF);
			if ((d >> 8) != fontTransparentIndex)
				to_write |= (d & 0xFF00);
			else
				to_write |= (prev & 0xFF00);
			video_buffer[offset + j] = to_write;
		}
	}
}

void put_string(char *s, int x, int y, u16 *video_buffer, int erase) {
	int i = 0;
	while (s[i] != 0) {
		put_character(s[i], x, y, video_buffer, erase);
		x += 8;
		i++;
        cursor+=8;
	}
}

void flip_and_show(void) {
	if (current_video_buffer == VIDEO_BUFFER_1) {
		REGISTRO_DISPLAY |= BACK_BUFFER;
		current_video_buffer = VIDEO_BUFFER_0;
	}
	else {
		REGISTRO_DISPLAY &= ~BACK_BUFFER;
		current_video_buffer = VIDEO_BUFFER_1;
	}
}

#define  GET_X_CENTER(width)  (120 - ((width) * 4))
#define RGB16(r,g,b)  ((r) | ((g) << 5) | ((b) << 10))
#define  ERASE_SCREEN(scr)  memset(scr, background_color, 240 * 160)

void wait_for_vsync() {
	while (((volatile u16) REG_VCOUNT) != 160);
}

int main() {
    REG_TM2D= 0;          
    REG_TM2CNT= 0b0000000010000011;
    
    int i = 0;
    REGISTRO_DISPLAY = MODOVIDEO_4 | BGENABLE2;
    background_color = 0;
    int toggle = 0;
    for (i = 0; i < 256; i++)
		BG_PALETTE_MEM[i] = fontPalette[i];

    put_string("Hola Mundo", 5, 10, current_video_buffer, 0);    
    while (1)
    {
        
        if (REG_TM2D >= 0x2000){
            REG_TM2CNT= 0b0000000000000011;
            REG_TM2CNT= 0b0000000010000011;
            if (toggle){
                put_character('_', cursor+8, 10, current_video_buffer, 0);
                toggle = 0;
            } else{
                put_character(' ', cursor+8, 10, current_video_buffer, 1);
                toggle = 1;
            }
            
        }
        wait_for_vsync();
    }
    return 0;
}
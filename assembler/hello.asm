.PLACE 0x2900 "Hello World!" 

; Font data for the 8x8 font
.PLACE 0x3100 [
0x0000, 0x0000, 0x0000, 0x0000, 0x3078, 0x7830, 0x3000, 0x3000, 
0x6c6c, 0x6c00, 0x0000, 0x0000, 0x6c6c, 0xfe6c, 0xfe6c, 0x6c00, 
0x307c, 0xc078, 0x0cf8, 0x3000, 0x00c6, 0xcc18, 0x3066, 0xc600, 
0x386c, 0x3876, 0xdccc, 0x7600, 0x6060, 0xc000, 0x0000, 0x0000, 
0x1830, 0x6060, 0x6030, 0x1800, 0x6030, 0x1818, 0x1830, 0x6000, 
0x0066, 0x3cff, 0x3c66, 0x0000, 0x0030, 0x30fc, 0x3030, 0x0000, 
0x0000, 0x0000, 0x0030, 0x3060, 0x0000, 0x00fc, 0x0000, 0x0000, 
0x0000, 0x0000, 0x0030, 0x3000, 0x060c, 0x1830, 0x60c0, 0x8000, 
0x7cc6, 0xcede, 0xf6e6, 0x7c00, 0x3070, 0x3030, 0x3030, 0xfc00, 
0x78cc, 0x0c38, 0x60cc, 0xfc00, 0x78cc, 0x0c38, 0x0ccc, 0x7800, 
0x1c3c, 0x6ccc, 0xfe0c, 0x1e00, 0xfcc0, 0xf80c, 0x0ccc, 0x7800, 
0x3860, 0xc0f8, 0xcccc, 0x7800, 0xfccc, 0x0c18, 0x3030, 0x3000, 
0x78cc, 0xcc78, 0xcccc, 0x7800, 0x78cc, 0xcc7c, 0x0c18, 0x7000, 
0x0030, 0x3000, 0x0030, 0x3000, 0x0030, 0x3000, 0x3030, 0x6000, 
0x1830, 0x60c0, 0x6030, 0x1800, 0x0000, 0xfc00, 0x00fc, 0x0000, 
0x6030, 0x180c, 0x1830, 0x6000, 0x78cc, 0x0c18, 0x3000, 0x3000, 
0x7cc6, 0xdede, 0xdec0, 0x7800, 0x3078, 0xcccc, 0xfccc, 0xcc00, 
0xfc66, 0x667c, 0x6666, 0xfc00, 0x3c66, 0xc0c0, 0xc066, 0x3c00, 
0xf86c, 0x6666, 0x666c, 0xf800, 0xfe62, 0x6878, 0x6862, 0xfe00, 
0xfe62, 0x6878, 0x6860, 0xf000, 0x3c66, 0xc0c0, 0xce66, 0x3e00, 
0xcccc, 0xccfc, 0xcccc, 0xcc00, 0x7830, 0x3030, 0x3030, 0x7800, 
0x1e0c, 0x0c0c, 0xcccc, 0x7800, 0xe666, 0x6c78, 0x6c66, 0xe600, 
0xf060, 0x6060, 0x6266, 0xfe00, 0xc6ee, 0xfefe, 0xd6c6, 0xc600, 
0xc6e6, 0xf6de, 0xcec6, 0xc600, 0x386c, 0xc6c6, 0xc66c, 0x3800, 
0xfc66, 0x667c, 0x6060, 0xf000, 0x78cc, 0xcccc, 0xdc78, 0x1c00, 
0xfc66, 0x667c, 0x6c66, 0xe600, 0x78cc, 0xe070, 0x1ccc, 0x7800, 
0xfcb4, 0x3030, 0x3030, 0x7800, 0xcccc, 0xcccc, 0xcccc, 0xfc00, 
0xcccc, 0xcccc, 0xcc78, 0x3000, 0xc6c6, 0xc6d6, 0xfeee, 0xc600, 
0xc6c6, 0x6c38, 0x386c, 0xc600, 0xcccc, 0xcc78, 0x3030, 0x7800, 
0xfec6, 0x8c18, 0x3266, 0xfe00, 0x7860, 0x6060, 0x6060, 0x7800, 
0xc060, 0x3018, 0x0c06, 0x0200, 0x7818, 0x1818, 0x1818, 0x7800, 
0x1038, 0x6cc6, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x00ff, 
0x3030, 0x1800, 0x0000, 0x0000, 0x0000, 0x780c, 0x7ccc, 0x7600, 
0xe060, 0x607c, 0x6666, 0xdc00, 0x0000, 0x78cc, 0xc0cc, 0x7800, 
0x1c0c, 0x0c7c, 0xcccc, 0x7600, 0x0000, 0x78cc, 0xfcc0, 0x7800, 
0x386c, 0x60f0, 0x6060, 0xf000, 0x0000, 0x76cc, 0xcc7c, 0x0cf8, 
0xe060, 0x6c76, 0x6666, 0xe600, 0x3000, 0x7030, 0x3030, 0x7800, 
0x0c00, 0x0c0c, 0x0ccc, 0xcc78, 0xe060, 0x666c, 0x786c, 0xe600, 
0x7030, 0x3030, 0x3030, 0x7800, 0x0000, 0xccfe, 0xfed6, 0xc600, 
0x0000, 0xf8cc, 0xcccc, 0xcc00, 0x0000, 0x78cc, 0xcccc, 0x7800, 
0x0000, 0xdc66, 0x667c, 0x60f0, 0x0000, 0x76cc, 0xcc7c, 0x0c1e, 
0x0000, 0xdc76, 0x6660, 0xf000, 0x0000, 0x7cc0, 0x780c, 0xf800, 
0x1030, 0x7c30, 0x3034, 0x1800, 0x0000, 0xcccc, 0xcccc, 0x7600, 
0x0000, 0xcccc, 0xcc78, 0x3000, 0x0000, 0xc6d6, 0xfefe, 0x6c00, 
0x0000, 0xc66c, 0x386c, 0xc600, 0x0000, 0xcccc, 0xcc7c, 0x0cf8, 
0x0000, 0xfc98, 0x3064, 0xfc00, 0x1c30, 0x30e0, 0x3030, 0x1c00, 
0x1818, 0x1800, 0x1818, 0x1800, 0xe030, 0x301c, 0x3030, 0xe000, 
0x76dc, 0x0000, 0x0000, 0x0000
]

@define DISP #0x3000 ; Display address
@define SP #0x0200 ; Stack pointer

.main: 
    LOAD SP, %SP ; Load the stack pointer
    LOAD DISP, %DISP ; Load the display address


    HLT ; Halt the program

const uint8_t SymbolMono18pt7bBitmaps[] PROGMEM = {
/*| 8 4 2 1 8 4 2 1 8 4 2 1 8 4 2 1 |*/
/*| . . . X X . . , . . X X . . . . |*/  0x18,0x30,
/*| . X X X X X . , . X X X X X . . |*/  0x7c,0x7c,
/*| X X X X X X X , X X X X X X X . |*/  0xfe,0xfe, 
/*| X X X X X X X X X X X X X X X . |*/  0xff,0xfe,
/*| X X X X X X X X X X X X X X X . |*/  0xff,0xfe,
/*| . X X X X X X X X X X X X X . . |*/  0x7f,0xfc,
/*| . X X X X X X X X X X X X X . . |*/  0x7f,0xfc,
/*| . . X X X X X X X X X X X . . . |*/  0x3f,0xf8,
/*| . . X X X X X X X X X X X . . . |*/  0x3f,0xf8,
/*| . . . X X X X X X X X X . . . . |*/  0x1f,0xf0,
/*| . . . . X X X X X X X . . . . . |*/  0x0f,0xe0,
/*| . . . . . X X X X X . . . . . . |*/  0x07,0xc0,
/*| . . . . . . X X X . . . . . . . |*/  0x03,0x80,
/*| . . . . . . . X . . . . . . . . |*/  0x01,0x00,
0x00
};//One more byte just in case

const GFXglyph SymbolMono18pt7bGlyphs[] PROGMEM = {
  //Index,  W, H,xAdv,dX, dY
  {  0, 16,14, 21, 3,-15}  //136 heart suit
};//144
  //Index,  W, H,xAdv,dX, dY
const GFXfont SymbolMono18pt7b PROGMEM = {
  (uint8_t  *)SymbolMono18pt7bBitmaps,
  (GFXglyph *)SymbolMono18pt7bGlyphs,
  0,48, 35 //ASCII start, ASCII stop,y Advance
};
#define GLYPH_HEART 0
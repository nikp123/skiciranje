#include "shared.h"
extern int loadLanguage(char *language);
extern SDL_Color translate_color(Uint32 int_color);

// mask 1 byte of 32bit integer
#define a32(a) ((a)>>24&0xFF)
#define r32(a) ((a)>>16&0xFF)
#define g32(a) ((a)>>8&0xFF)
#define b32(a) ((a)&0xFF)

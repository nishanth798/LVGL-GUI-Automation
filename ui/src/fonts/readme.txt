fonts generated using font converter at https://lvgl.io/tools/fontconverter

font source: https://fonts.google.com/specimen/Open+Sans
click: "Download Family"

example configuration for "opensans_bold_40"

Name: opensans_bold_40
Size: 40
Bpp: 4 bit-per-pixel
TTF/WOFF file: Open_Sans/static/OpenSans-Bold.ttf
Range: 0x20-0x7F (filters ascii characters)



don't forget to declare the newly added fonts in fonts.h as follows 

LV_FONT_DECLARE(opensans_bold_40);

If the following lines are not present in the created .c file add them in the top of the file
#ifndef OPENSANS_BOLD_40
#define OPENSANS_BOLD_40 1
#endif

comment the following line in the descriptor which will be present at the end of the file.

//.static_bitmap = 0,
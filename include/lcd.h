#ifndef __LCD_H__
#define __LCD_H__

#define CHAR_RIGHT_ARROW 126
#define CHAR_LEFT_ARROW 127

void set_line_text(int i_line, char *pc_text);
void set_char(int i_x, int i_y, char c_char);
void *lcd_loop(void *data);

#endif


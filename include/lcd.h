#ifndef __LCD_H__
#define __LCD_H__

#define CHAR_FULL 255
#define CHAR_RIGHT_ARROW 126
#define CHAR_LEFT_ARROW 127
#define CHAR_BELL 1
#define CHAR_PLAY 2
#define CHAR_PAUSE 3
#define CHAR_STOP 4
#define CHAR_PREV 5
#define CHAR_NEXT 6
#define CHAR_SPEAKER 7

void  lcd_set_line_text(int i_line, char *pc_text, int i_flush);
void  lcd_set_char(int i_x, int i_y, char c_char);
void *lcd_loop(void *data);

#endif


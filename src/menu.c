#include <string.h>
#include "lcd.h"
#include "main.h"
#include "input.h"

char *ppc_main_menu[21] =
{
    "set alarm",
    "reboot",
    "poweroff",
    "\0"
};

char **ppc_menu = ppc_main_menu;
int    i_current_item = 0;

void menu_draw(void)
{
    char pc_text[21];

    memset(pc_text, 255, 20);
    pc_text[20] = '\0';
    pc_text[8] = 'M';
    pc_text[9] = 'E';
    pc_text[10] = 'N';
    pc_text[11] = 'U';
    lcd_set_line_text(0, pc_text, TRUE);

    lcd_set_line_text(1, i_current_item > 0 ? ppc_menu[i_current_item - 1] : NULL, TRUE);
    lcd_set_line_text(2, ppc_menu[i_current_item], TRUE);
    lcd_set_line_text(3, ppc_menu[i_current_item + 1][0] != '\0' ? ppc_menu[i_current_item + 1] : NULL, TRUE);
}

int menu_input(int i_keycode)
{
    int i_end = FALSE;

    switch(i_keycode)
    {
        case BTN_BACK:
            if(ppc_menu == ppc_main_menu)
            {
                i_end = TRUE;
            }
            break;
        case BTN_UP:
            if(i_current_item > 0) i_current_item--;
            break;
        case BTN_DOWN:
            if(ppc_menu[i_current_item + 1][0] != '\0') i_current_item++;
            break;
    }

    return i_end;
}



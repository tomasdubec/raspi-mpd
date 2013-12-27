#ifndef __INPUT_H__
#define __INPUT_H__

#define BTN_UP   4
#define BTN_DOWN 5
#define BTN_BACK 6
#define BTN_OK   7

#define INPUT_LOOP_SLEEP 100000

void *input_loop(void *data);

void (*p_btn_depressed)(int);
void (*p_btn_pressed)(int);

#endif

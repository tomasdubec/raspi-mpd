#include <unistd.h>
#include <pthread.h>
#include <wiringPi.h>
#include "logging.h"
#include "input.h"
#include "main.h"

void init_inputs(void)
{
    LOG_DEBUG("Initializing GPIO input buttons");

    pinMode(BTN_UP, INPUT);
    pullUpDnControl(BTN_UP, PUD_UP);
    pinMode(BTN_DOWN, INPUT);
    pullUpDnControl(BTN_DOWN, PUD_UP);
    pinMode(BTN_BACK, INPUT);
    pullUpDnControl(BTN_BACK, PUD_UP);
    pinMode(BTN_OK, INPUT);
    pullUpDnControl(BTN_OK, PUD_UP);
}

void *input_loop(void *data)
{
    int pi_last_state[8] = {1, 1, 1, 1, 1, 1, 1, 1};
    char *ps_button_names[8] = {"", "", "", "", "UP", "DOWN", "BACK", "OK"};
    int i_tmp;
    int i_index;

    LOG_DEBUG("Input thread starting");

    p_btn_depressed = NULL;

    init_inputs();

    while(!b_end)
    {
        for(i_index = BTN_UP;i_index <= BTN_OK;i_index++)
        {
            i_tmp = digitalRead(i_index);
            if(i_tmp != pi_last_state[i_index])
            {
                if(i_tmp == 1)
                {
                    if(p_btn_depressed != NULL) p_btn_depressed(i_index);
                }
                else
                {
                    if(p_btn_pressed != NULL) p_btn_pressed(i_index);
                }
                pi_last_state[i_index] = i_tmp;
                LOG_DEBUG("Button %s was %spressed", ps_button_names[i_index], i_tmp == 1 ? "de" : "");
            }
        }

        usleep(INPUT_LOOP_SLEEP);
    }

    LOG_DEBUG("Input thread quitting");

    return NULL;
}


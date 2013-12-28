#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPi.h>
#include "logging.h"
#include "input.h"
#include "main.h"

extern pthread_mutex_t r_input_queue_mutex;
extern queue_t *pr_input_queue;

void input_push(int i_keycode);

void input_init(void)
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

    p_btn_pressed = input_push;
}

void input_push(int i_keycode)
{
    queue_t *pr_tmp;
    queue_t *pr_iter;

    LOG_DEBUG2("Locking input queue mutex");
    pthread_mutex_lock(&r_input_queue_mutex);

    LOG_DEBUG("Pushing keycode %d into input queue", i_keycode);

    pr_tmp = (queue_t*)calloc(sizeof(queue_t), 1);
    pr_tmp->pr_next = NULL;
    pr_tmp->i_keycode = i_keycode;

    if(pr_input_queue == NULL)
    {
        pr_input_queue = pr_tmp;
    }
    else
    {
        pr_iter = pr_input_queue;
        while(pr_iter->pr_next != NULL)
            pr_iter = pr_iter->pr_next;

        pr_iter->pr_next = pr_tmp;
    }

    LOG_DEBUG2("Unlocking input queue mutex");
    pthread_mutex_unlock(&r_input_queue_mutex);
}

void *input_loop(void *data)
{
    int pi_last_state[8] = {1, 1, 1, 1, 1, 1, 1, 1};
    int i_tmp;
    int i_index;

    LOG_DEBUG("Input thread starting");

    p_btn_depressed = NULL;

    input_init();

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
                LOG_DEBUG("Button %d was %spressed", i_index, i_tmp == 1 ? "de" : "");
            }
        }

        usleep(INPUT_LOOP_SLEEP);
    }

    LOG_DEBUG("Input thread quitting");

    return NULL;
}


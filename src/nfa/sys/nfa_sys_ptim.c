/*****************************************************************************
**
**  Name:           nfa_sys_ptim.c
**
**  Description:    Protocol timer services (taken from bta ptim)
**
**  Copyright (c) 2010, Broadcom Corp., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#include "nfc_target.h"
#include "gki.h"
#include "nfa_sys_ptim.h"
#include "nfa_sys.h"
#include "nfa_sys_int.h"

/*******************************************************************************
**
** Function         nfa_sys_ptim_init
**
** Description      Initialize a protocol timer control block.  Parameter
**                  period is the GKI timer period in milliseconds.  Parameter
**                  timer_id is the GKI timer id.
**
** Returns          void
**
*******************************************************************************/
void nfa_sys_ptim_init (tPTIM_CB *p_cb, UINT16 period, UINT8 timer_id)
{
    GKI_init_timer_list (&p_cb->timer_queue);
    p_cb->period = period;
    p_cb->timer_id = timer_id;
}

/*******************************************************************************
**
** Function         nfa_sys_ptim_timer_update
**
** Description      Update the protocol timer list and handle expired timers.
**                  This function is called from the task running the protocol
**                  timers when the periodic GKI timer expires.
**
** Returns          void
**
*******************************************************************************/
void nfa_sys_ptim_timer_update (tPTIM_CB *p_cb)
{
    TIMER_LIST_ENT *p_tle;
    BT_HDR *p_msg;
    UINT32 new_ticks_count;
    INT32  period_in_ticks;

    /* To handle the case when the function is called less frequently than the period
       we must convert determine the number of ticks since the last update, then
       convert back to milliseconds before updating timer list */
    new_ticks_count = GKI_get_tick_count ();

    /* Check for wrapped condition */
    if (new_ticks_count >= p_cb->last_gki_ticks)
    {
        period_in_ticks = (INT32) (new_ticks_count - p_cb->last_gki_ticks);
    }
    else
    {
        period_in_ticks = (INT32) (((UINT32) 0xffffffff - p_cb->last_gki_ticks)
                            + new_ticks_count + 1);
    }

    /* update timer list */
    GKI_update_timer_list (&p_cb->timer_queue, GKI_TICKS_TO_MS (period_in_ticks));

    p_cb->last_gki_ticks = new_ticks_count;

    /* while there are expired timers */
    while ((p_cb->timer_queue.p_first) && (p_cb->timer_queue.p_first->ticks <= 0))
    {
        /* removed expired timer from list */
        p_tle = p_cb->timer_queue.p_first;
        NFA_TRACE_DEBUG1 ("nfa_sys_ptim_timer_update expired: %08x", p_tle);
        GKI_remove_from_timer_list (&p_cb->timer_queue, p_tle);

        /* call timer callback */
        if (p_tle->p_cback)
        {
            (*p_tle->p_cback) (p_tle);
        }
        else if (p_tle->event)
        {
            if ((p_msg = (BT_HDR *) GKI_getbuf (sizeof (BT_HDR))) != NULL)
            {
                p_msg->event = p_tle->event;
                p_msg->layer_specific = 0;
                nfa_sys_sendmsg (p_msg);
            }
        }
    }

    /* if timer list is empty stop periodic GKI timer */
    if (p_cb->timer_queue.p_first == NULL)
    {
        NFA_TRACE_DEBUG0 ("ptim timer stop");
        GKI_stop_timer (p_cb->timer_id);
    }
}

/*******************************************************************************
**
** Function         nfa_sys_ptim_start_timer
**
** Description      Start a protocol timer for the specified amount
**                  of time in seconds.
**
** Returns          void
**
*******************************************************************************/
void nfa_sys_ptim_start_timer (tPTIM_CB *p_cb, TIMER_LIST_ENT *p_tle, UINT16 type, INT32 timeout)
{
    NFA_TRACE_DEBUG1 ("nfa_sys_ptim_start_timer %08x", p_tle);

    /* if timer list is currently empty, start periodic GKI timer */
    if (p_cb->timer_queue.p_first == NULL)
    {
        NFA_TRACE_DEBUG0 ("ptim timer start");
        p_cb->last_gki_ticks = GKI_get_tick_count ();
        GKI_start_timer (p_cb->timer_id, GKI_MS_TO_TICKS (p_cb->period), TRUE);
    }

    GKI_remove_from_timer_list (&p_cb->timer_queue, p_tle);

    p_tle->event = type;
    p_tle->ticks = timeout;

    GKI_add_to_timer_list (&p_cb->timer_queue, p_tle);
}

/*******************************************************************************
**
** Function         nfa_sys_ptim_stop_timer
**
** Description      Stop a protocol timer.
**
** Returns          void
**
*******************************************************************************/
void nfa_sys_ptim_stop_timer (tPTIM_CB *p_cb, TIMER_LIST_ENT *p_tle)
{
    NFA_TRACE_DEBUG1 ("nfa_sys_ptim_stop_timer %08x", p_tle);

    GKI_remove_from_timer_list (&p_cb->timer_queue, p_tle);

    /* if timer list is empty stop periodic GKI timer */
    if (p_cb->timer_queue.p_first == NULL)
    {
        NFA_TRACE_DEBUG0 ("ptim timer stop");
        GKI_stop_timer (p_cb->timer_id);
    }
}

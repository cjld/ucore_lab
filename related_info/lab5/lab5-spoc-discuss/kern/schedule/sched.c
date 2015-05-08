#include <list.h>
#include <sync.h>
#include <proc.h>
#include <sched.h>
#include <assert.h>

void
wakeup_proc(struct proc_struct *proc) {
    assert(proc->state != PROC_ZOMBIE);
    bool intr_flag;
    local_intr_save(intr_flag);
    {
        if (proc->state != PROC_RUNNABLE) {
            proc->state = PROC_RUNNABLE;
            proc->wait_state = 0;
        }
        else {
            warn("wakeup runnable process.\n");
        }
    }
    local_intr_restore(intr_flag);
}

const char * get_state_str(enum proc_state state) {
#define ck_state(STATE) \
	if (state == STATE) \
		return "'"#STATE"'";

	ck_state(PROC_UNINIT);
	ck_state(PROC_SLEEPING);
	ck_state(PROC_RUNNABLE);
	ck_state(PROC_ZOMBIE);
}

void
schedule(void) {
    bool intr_flag;
    list_entry_t *le, *last;
    struct proc_struct *next = NULL;
    local_intr_save(intr_flag);
    {
    	struct proc_struct* tmp = current;
    	cprintf("schedule proc %d, state %s\n", current->pid,
    			get_state_str(current->state));
        current->need_resched = 0;
        last = (current == idleproc) ? &proc_list : &(current->list_link);
        le = last;
        do {
            if ((le = list_next(le)) != &proc_list) {
                next = le2proc(le, list_link);
                if (next->state == PROC_RUNNABLE) {
                    break;
                }
            }
        } while (le != last);
        if (next == NULL || next->state != PROC_RUNNABLE) {
            next = idleproc;
        }
        next->runs ++;
        if (next != current) {
            proc_run(next);
        	cprintf("my state become %s\n", get_state_str(tmp->state));
        	cprintf("switch proc %d, state %s, run time %d\n",
        			next->pid, get_state_str(next->state), next->runs);
        } else {
        	cprintf("my state become %s\n", get_state_str(tmp->state));
        	cprintf("continue run proc %d, run time %d\n",
        			current->pid, current->runs);
        }
        cprintf("\n");
    }
    local_intr_restore(intr_flag);
}


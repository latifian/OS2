/*-
 * Copyright (c) 1999 Assar Westerlund
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: release/10.0.0/share/examples/kld/syscall/module/syscall.c 193374 2009-06-03 09:28:58Z pjd $
 */

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysproto.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/sched.h>

int get_tslice(struct thread* td);
struct thread* mohlat_choose(void);
void my_thread_timeout(struct thread* td);

struct thread* queue1[10000], *queue2[10000], *queue3[10000];
int start1, end1, start2, end2, start3, end3;

int get_tslice(struct thread* td) {
        return 1<<(td->qnum);
}
void my_thread_timeout(struct thread* td) {
        if (td->qnum == 1) {
                td->qnum = 2;
                queue2[end1++] = td;
                end1 %= 9999;
        }
        else if (td->qnum == 2) {
                td->qnum = 3;
                queue3[end2++] = td;
                end2 %= 9999;
        } else {
                td->qnum = 3;
                queue3[end3++] = td;
                end3 %= 9999;
        }
        
}
struct thread* mohlat_choose() {
        start1 %= 9999;
        start2 %= 9999;
        start3 %= 9999;
        if (start1  != end1) {
                return queue1[start1++];
                
        }
        if (start2 != end2) {
                return queue2[start2++];
        }
        if (start3 != end3) {
                return queue3[start3++];
        }
        return NULL;
}



/*
 * The function for implementing the syscall.
 */
static int
hello(struct thread *td, void *arg)
{
        td->ismine = 1;
        td->qnum = 1;
        queue1[end1++]=td;
        end1 %= 9999;
	printf("hello kernel\n");
	return (0);
}

/*
 * The `sysent' for the new syscall
 */
static struct sysent hello_sysent = {
	0,			/* sy_narg */
	hello			/* sy_call */
};

/*
 * The offset in sysent where the syscall is allocated.
 */
static int offset = NO_SYSCALL;

/*
 * The function called at load/unload.
 */
static int
load(struct module *module, int cmd, void *arg)
{
	int error = 0;

	switch (cmd) {
	case MOD_LOAD :
                start1 = end1 = start2 = end2 = start3 = end3 = 0;
		printf("syscall loaded at %d\n", offset);
                //set_get_tslice(&get_tslice);
                set_my_thread_timeout(&my_thread_timeout);
                set_mohlat_choose(&mohlat_choose);
		break;
	case MOD_UNLOAD :
                //set_get_tslice(NULL);
                set_my_thread_timeout(NULL);
                set_mohlat_choose(NULL);
		printf("syscall unloaded from %d\n", offset);
		break;
	default :
		error = EOPNOTSUPP;
		break;
	}
	return (error);
}

SYSCALL_MODULE(syscall, &offset, &hello_sysent, load, NULL);

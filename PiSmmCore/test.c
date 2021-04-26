#include <linux/sched.h>
#include <linux/init.h>
#include <linux/list.h>
#include <stdio.h>

int main(){
    struct task_struct *p = &init_task;
    for_each_process(p)
        printf("process:%s(pid %i)",p->comm,p->pid);
    
    /*
        0 = all
        1 = active
        2 = inactive
    */

    //print("%d",conn.listAllDomains(flag));

    /*
    for i in range (len(domList)):
        domain = domList[i]
        print(domain.name())
        print(domain.UUIDString())
        threads = libvirt_qemu.qemuMonitorCommand(domain, 'info cpus', 1)
        print(threads)
    */
}

    
#include<linux/init.h>
#include<linux/module.h>
#include<linux/mm.h>
#include<linux/mm_types.h>
#include<linux/sched.h>
#include<linux/export.h>
#include<linux/init_task.h>

static unsigned long cr0,cr3;
static unsigned long vaddr=0;

#define SMBASE 0xB83C6000
#define PTE_PFN_MASK1 0xffffffffff000
#define PAGE_MASK1    0xfffffffffffff000
#define PAGE_OFFSET1  0xffff963140000000

//define a fix address
#define NEW_MAP_PTE 0x111110000


static void get_pgtable_macro(void)
{
    printk("PGDIR_SHIFT=%d\n",PGDIR_SHIFT);
    printk("P4D_SHIFT=%d\n",P4D_SHIFT);
    printk("PUD_SHIFT=%d\n",PUD_SHIFT);
    printk("PMD_SHIFT=%d\n",PMD_SHIFT);
    printk("PAGE_SHIFT=%d\n",PAGE_SHIFT);
    printk("PAGE_OFFSET=%Lx\n",PAGE_OFFSET);
    printk("PTRS_PER_PGD=%d\n",PTRS_PER_PGD);
    printk("PTRS_PER_P4D=%d\n",PTRS_PER_P4D);
    printk("PTRS_PER_PUD=%d\n",PTRS_PER_PUD);
    printk("PTRS_PER_PMD=%d\n",PTRS_PER_PMD);
    printk("PTRS_PER_PTE=%d\n",PTRS_PER_PTE);
    printk("PAGE_MASK=0x%lx\n",PAGE_MASK);
}


static unsigned long handler0(unsigned long vaddr, unsigned long cr3){
    unsigned long paddr=0;
    unsigned long page_addr=0;
    unsigned long P_OFFSET=0;
    unsigned long *pgd, *pud, *pmd, *pte;
    unsigned long *pte_c;
    unsigned long pte_mask;


    pgd = (unsigned long *)(cr3);
    pgd = pgd + ((vaddr>>39) & 0x1FF);
    printk("pgd = 0x%lx *pgd = 0x%lx  \n", pgd, *pgd);

    pud = (unsigned long *)(((unsigned long)*pgd & PTE_PFN_MASK) + PAGE_OFFSET);
//  printk("*pgd & PTE_PFN_MASK + PAGE_OFFSET = 0x%lx  \n", ((unsigned long)*pgd & PTE_PFN_MASK) + PAGE_OFFSET);
    pud = pud + ((vaddr>>30) & 0x1FF);
//    printk("pud = 0x%lx  \n", pud);
   
    printk("pud = 0x%lx *pud = 0x%lx  \n", pud, *pud);
    

    pmd = (unsigned long *)(((unsigned long)*pud & PTE_PFN_MASK) + PAGE_OFFSET);
    pmd = pmd + ((vaddr>>21) & 0x1FF);
    printk("pmd = 0x%lx *pmd = 0x%lx  \n", pmd, *pmd);

    pte = (unsigned long *)(((unsigned long)*pmd & PTE_PFN_MASK) + PAGE_OFFSET);
    pte = pte + ((vaddr>>12) & 0x1FF);
    printk("pte = 0x%lx *pte = 0x%lx  \n", pte, *pte);

    page_addr= (*pte) & PAGE_MASK1;
    P_OFFSET=vaddr&~PAGE_MASK1;
    paddr=page_addr|P_OFFSET;
    printk("page_addr=0x%lx,page_offset=0x%lx\n",page_addr,P_OFFSET);
    printk("vaddr=0x%lx,paddr=0x%lx\n",vaddr,paddr);
 //   __asm__ volatile("rsm":::);
    return paddr;
}


static int __init v2p_init(void)
{
    unsigned long vaddr=0;
    printk("vaddr to paddr module is running..\n");
    vaddr = 0xffffffffc0a13000;
    cr3  = 0x10cd8a000;
    handler0(vaddr, cr3)
    return 0;
}

static void __exit v2p_exit(void)
{
    printk("vaddr to paddr module is leaving..\n");
    free_page(vaddr);
}

module_init(v2p_init);
module_exit(v2p_exit);


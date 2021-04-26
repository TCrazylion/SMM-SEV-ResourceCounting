#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0x9de7765d, "module_layout" },
	{ 0x4302d0eb, "free_pages" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0x1d19f77b, "physical_mask" },
	{ 0x5367b4b4, "boot_cpu_data" },
	{ 0x7dad3299, "init_task" },
	{ 0xdad13544, "ptrs_per_p4d" },
	{ 0x72d79d83, "pgdir_shift" },
	{ 0x8a35b432, "sme_me_mask" },
	{ 0x67c0c54c, "pv_ops" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "3E997F64F705E32DAE7435A");

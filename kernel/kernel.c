#include "multiboot2.h"
#include "printk.h"
#include "kprint.h"
#include "mm.h"
#include "irq.h"
#include "task.h"
#include "gate.h"
#include "ata.h"

extern global_mm_descriptor_t gmdsc;
extern multiboot_uint64_t mb2_magic;
extern multiboot_uint64_t mb2_info;
static void parse_sys_info(multiboot_uint64_t mb2_info_addr);

int kernel_start() {
    if (mb2_magic != MULTIBOOT2_BOOTLOADER_MAGIC || mb2_info & 7) {
        return -1;
    }
    // 清屏
    clear_screen();
    load_TR(10);
    // 解析系统信息
    parse_sys_info(mb2_info);

    // 初始化内存
    init_memory();
    // mm_page_t *page;
    // page = alloc_pages(ZONE_NORMAL, 64, PG_PTable_Maped|PG_Active|PG_Kernel);
    // for (int i = 0; i <= 64; i++) {
    //     kdebug("page:%d  attr:%x  addr:%x\n", i, (page+i)->attribute, (page+i)->phy_addr);
    // }
    // uint_t *gdt = get_gdt();

    // 初始化中断
    init_interrupt();

    ata_init();

    task_init();
    
    while (1) ;
    return 0;
}

static void _init_memory(struct multiboot_tag *tag);

static void parse_sys_info(multiboot_uint64_t mb2_info_addr) {
    struct multiboot_tag *tag;
    for (tag = (struct multiboot_tag*)(mb2_info_addr + MULTIBOOT_TAG_ALIGN);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag*)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7))) {
        switch (tag->type) {
        case MULTIBOOT_TAG_TYPE_MMAP:
            _init_memory(tag);
            break;
        default:
            break;
        }
    }
}

static void _init_memory(struct multiboot_tag *tag) {
    struct multiboot_tag_mmap *mmap = (struct multiboot_tag_mmap*) tag;
    struct multiboot_mmap_entry *entry = mmap->entries;

    uint32_t i = 0, available_size = 0;
    while ((uint8_t*)entry < (uint8_t*)mmap + mmap->size) {
        gmdsc.e820[i].addr = entry->addr;
        gmdsc.e820[i].len = entry->len;
        gmdsc.e820[i].type = entry->type;
        kprintf("[info] e820 memory: addr:%x len:%d type:%d\n", gmdsc.e820[i].addr, gmdsc.e820[i].len, gmdsc.e820[i].type);
        if (gmdsc.e820[i].type == E820_TYPE_AVAILABLE) {
            available_size++;
        }
        i++;
        
        entry = (struct multiboot_mmap_entry*)((uint8_t*)entry + mmap->entry_size);
    }
    gmdsc.e820_num = i;
    gmdsc.e820_available_size = available_size;
}
#ifndef MEM_H
#define MEM_H
#define __fram __attribute__((section(".persistent")))///GNU compiler
//#define __fram __attribute__((section(".TI.persistent")))///TI compiler
/*
#define __ro_fram __attribute__((section(".persistent")))
#define __hifram __attribute__((section(".persistent_hifram")))
#define __ro_hifram __attribute__((section(".persistent_hifram")))*/
//#define __known __attribute__((section(".known")))

#endif

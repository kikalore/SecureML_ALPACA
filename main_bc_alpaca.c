#include <msp430.h>
#include <stdlib.h>
#include <libalpaca/alpaca.h>
#include <libmsp/mem.h>
#include <libmsp/watchdog.h>
#include <libPF/PF_sim.h>

#define SEED 4L
#define ITER 100
#define CHAR_BIT 8
#define LIBIO_BACKEND_HWUART


__nv static char bits[256] =
{
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,  /* 0   - 15  */
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 16  - 31  */
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 32  - 47  */
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 48  - 63  */
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 64  - 79  */
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 80  - 95  */
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 96  - 111 */
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 112 - 127 */
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 128 - 143 */
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 144 - 159 */
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 160 - 175 */
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 176 - 191 */
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 192 - 207 */
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 208 - 223 */
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 224 - 239 */
	4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8   /* 240 - 255 */
};
GLOBAL_SB(unsigned, n_0);
GLOBAL_SB(unsigned, n_1);
GLOBAL_SB(unsigned, n_2);
GLOBAL_SB(unsigned, n_3);
GLOBAL_SB(unsigned, n_4);
GLOBAL_SB(unsigned, n_5);
GLOBAL_SB(unsigned, n_6);

GLOBAL_SB(unsigned, func);

GLOBAL_SB(uint32_t, seed);
GLOBAL_SB(unsigned, iter);

#define MEM_SIZE 0x4
__nv uint8_t *data_src[MEM_SIZE];
__nv uint8_t *data_dest[MEM_SIZE];
__nv unsigned int data_size[MEM_SIZE];
__nv uint8_t exacution_counter=0;
void clear_isDirty() {}

void init();
void task_init();
void task_select_func();
void task_bit_count();
void task_bitcount();
void task_ntbl_bitcnt();
void task_ntbl_bitcount();
void task_BW_btbl_bitcount();
void task_AR_btbl_bitcount();
void task_bit_shifter();
void task_end();

	TASK(1, task_init)
	TASK(2, task_select_func)
	TASK(3, task_bit_count)
	TASK(4, task_bitcount)
	TASK(5, task_ntbl_bitcnt)
	TASK(6, task_ntbl_bitcount)
	TASK(7, task_BW_btbl_bitcount)
	TASK(8, task_AR_btbl_bitcount)
	TASK(9, task_bit_shifter)
	TASK(10, task_end)

unsigned overflow=0;

static void init_hw()
{
	msp_watchdog_disable();
	// PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode
	// P3DIR ^= 0xFF;
	//   P3OUT ^=0x04;
	//   P3OUT ^=0x04;
	//msp_gpio_unlock();
	//msp_clock_setup();
}

void init() {

	init_hw();
	PF_sim_start();
	//INIT_CONSOLE();

	__enable_interrupt();

}

void task_init() {

    if(exacution_counter>100){
        while(1);
    }
	GV(func) = 0;
	GV(n_0) = 0;
	GV(n_1) = 0;
	GV(n_2) = 0;
	GV(n_3) = 0;
	GV(n_4) = 0;
	GV(n_5) = 0;
	GV(n_6) = 0;

	TRANSITION_TO(task_select_func);
}

void task_select_func() {


	GV(seed) = (uint32_t)SEED; // for test, seed is always the same
	GV(iter) = 0;

	if(GV(func) == 0){
		GV(func)++;
		TRANSITION_TO(task_bit_count);
	}
	else if(GV(func) == 1){
		GV(func)++;
		TRANSITION_TO(task_bitcount);
	}
	else if(GV(func) == 2){
		GV(func)++;
		TRANSITION_TO(task_ntbl_bitcnt);
	}
	else if(GV(func) == 3){
		GV(func)++;
		TRANSITION_TO(task_ntbl_bitcount);
	}
	else if(GV(func) == 4){
		GV(func)++;
		TRANSITION_TO(task_BW_btbl_bitcount);
	}
	else if(GV(func) == 5){
		GV(func)++;
		TRANSITION_TO(task_AR_btbl_bitcount);
	}
	else if(GV(func) == 6){
		GV(func)++;
		TRANSITION_TO(task_bit_shifter);
	}
	else{
		TRANSITION_TO(task_end);

	}
}
void task_bit_count() {

	uint32_t tmp_seed = GV(seed);
	GV(seed) = GV(seed) + 13;
	unsigned temp = 0;
	if(tmp_seed) do 
		temp++;
	while (0 != (tmp_seed = tmp_seed&(tmp_seed-1)));
	GV(n_0) += temp;
	GV(iter)++;

	if(GV(iter) < ITER){
		TRANSITION_TO(task_bit_count);
	}
	else{
		TRANSITION_TO(task_select_func);
	}
}
void task_bitcount() {

	uint32_t tmp_seed = GV(seed);
	GV(seed) = GV(seed) + 13;
	tmp_seed = ((tmp_seed & 0xAAAAAAAAL) >>  1) + (tmp_seed & 0x55555555L);
	tmp_seed = ((tmp_seed & 0xCCCCCCCCL) >>  2) + (tmp_seed & 0x33333333L);
	tmp_seed = ((tmp_seed & 0xF0F0F0F0L) >>  4) + (tmp_seed & 0x0F0F0F0FL);
	tmp_seed = ((tmp_seed & 0xFF00FF00L) >>  8) + (tmp_seed & 0x00FF00FFL);
	tmp_seed = ((tmp_seed & 0xFFFF0000L) >> 16) + (tmp_seed & 0x0000FFFFL);
	GV(n_1) += (int)tmp_seed;
	GV(iter)++;

	if(GV(iter) < ITER){
		TRANSITION_TO(task_bitcount);
	}
	else{
		TRANSITION_TO(task_select_func);
	}
}
int recursive_cnt(uint32_t x){
	int cnt = bits[(int)(x & 0x0000000FL)];

	if (0L != (x >>= 4))
		cnt += recursive_cnt(x);

	return cnt;
}
void task_ntbl_bitcnt() {

	GV(n_2) += recursive_cnt(GV(seed));	
	GV(seed) = GV(seed) + 13;
	GV(iter)++;

	if(GV(iter) < ITER){
		TRANSITION_TO(task_ntbl_bitcnt);
	}
	else{
		TRANSITION_TO(task_select_func);
	}
}
void task_ntbl_bitcount() {

	GV(n_3) += bits[ (int) (GV(seed) & 0x0000000FUL)       ] +
		bits[ (int)((GV(seed) & 0x000000F0UL) >> 4) ] +
		bits[ (int)((GV(seed) & 0x00000F00UL) >> 8) ] +
		bits[ (int)((GV(seed) & 0x0000F000UL) >> 12)] +
		bits[ (int)((GV(seed) & 0x000F0000UL) >> 16)] +
		bits[ (int)((GV(seed) & 0x00F00000UL) >> 20)] +
		bits[ (int)((GV(seed) & 0x0F000000UL) >> 24)] +
		bits[ (int)((GV(seed) & 0xF0000000UL) >> 28)];
	GV(seed) = GV(seed) + 13;
	GV(iter)++;

	if(GV(iter) < ITER){
		TRANSITION_TO(task_ntbl_bitcount);
	}
	else{
		TRANSITION_TO(task_select_func);
	}
}
void task_BW_btbl_bitcount() {

	union 
	{ 
		unsigned char ch[4]; 
		long y; 
	} U; 

	U.y = GV(seed); 

	GV(n_4) += bits[ U.ch[0] ] + bits[ U.ch[1] ] + 
		bits[ U.ch[3] ] + bits[ U.ch[2] ]; 
	GV(seed) = GV(seed) + 13;
	GV(iter)++;

	if(GV(iter) < ITER){
		TRANSITION_TO(task_BW_btbl_bitcount);
	}
	else{
		TRANSITION_TO(task_select_func);
	}
}
void task_AR_btbl_bitcount() {

	unsigned char * Ptr = (unsigned char *) &GV(seed) ;
	int Accu ;

	Accu  = bits[ *Ptr++ ];
	Accu += bits[ *Ptr++ ];
	Accu += bits[ *Ptr++ ];
	Accu += bits[ *Ptr ];
	GV(n_5)+= Accu;
	GV(seed) = GV(seed) + 13;
	GV(iter)++;

	if(GV(iter) < ITER){
		TRANSITION_TO(task_AR_btbl_bitcount);
	}
	else{
		TRANSITION_TO(task_select_func);
	}
}
void task_bit_shifter() {

	int i, nn;
	uint32_t tmp_seed = GV(seed);
	for (i = nn = 0; tmp_seed && (i < (sizeof(long) * CHAR_BIT)); ++i, tmp_seed >>= 1)
		nn += (int)(tmp_seed & 1L);
	GV(n_6) += nn;
	GV(seed) = GV(seed) + 13;

	GV(iter)++;

	if(GV(iter) < ITER){
		TRANSITION_TO(task_bit_shifter);
	}
	else{
		TRANSITION_TO(task_select_func);
	}
}

void task_end() {

    P3OUT ^= 0x01;
    P3OUT ^= 0x01;
    exacution_counter++;
	TRANSITION_TO(task_init);
}
ENTRY_TASK(task_init)
INIT_FUNC(init)

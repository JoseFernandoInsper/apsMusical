#include "asf.h"
#include "notas.h"
#include "Pinos.h"

volatile Bool but1_flag = false;
volatile Bool but2_flag = false;
volatile Bool but3_flag = false;

void BUT_init(void);
void LED_init(int estado);
void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq);
void pin_toggle(Pio *pio, uint32_t mask);

static void Button1_Handler(void)
{
	but1_flag = !but1_flag;
}
static void Button2_Handler(void)
{
	but2_flag = !but2_flag;
}
static void Button3_Handler(void)
{
	but3_flag = !but3_flag;
}

void TC1_Handler(void){
	volatile uint32_t ul_dummy;

	ul_dummy = tc_get_status(TC0, 1);

	UNUSED(ul_dummy);
	
	pin_toggle(LED_PIO, LED_PIN_MASK);
}

void TC0_Handler(void){
	volatile uint32_t ul_dummy;

	ul_dummy = tc_get_status(TC0, 0);

	UNUSED(ul_dummy);
	pin_toggle(BUZZER_PIO, BUZZER_PIO_IDX_MASK);	
}

void TC2_Handler(void){
	volatile uint32_t ul_dummy;

	ul_dummy = tc_get_status(TC0, 2);

	UNUSED(ul_dummy);

	pin_toggle(LED3_PIO, LED3_PIN_MASK);
}

void pause (void){
	pio_clear(LED3_PIO, LED3_PIN_MASK);
	tc_stop(TC0, 0);
	while(1) {
		if (but3_flag == false) {
			pio_set(LED3_PIO, LED3_PIN_MASK);
			return;
		}
	}
}

void buzzer(int freq, long duration) {
	TC_init(TC0, ID_TC0, 0, freq);
	pin_toggle(LED_PIO, LED_PIN_MASK);
	pin_toggle(LED2_PIO, LED2_PIN_MASK);
	delay_ms(duration);
}

void play(long music[], long duration[], int n_de_notas){
	for (int i= 0; i< n_de_notas; i++) {
		if(but3_flag == true) {
			pause();
		}
		buzzer(music[i], duration[i]);
	}
}

void pin_toggle(Pio *pio, uint32_t mask){
	if(pio_get_output_data_status(pio, mask))
	pio_clear(pio, mask);
	else
	pio_set(pio,mask);
}

void BUT_init(void){
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pmc_enable_periph_clk(BUT3_PIO_ID);
	pio_set_input(BUT1_PIO, BUT1_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_input(BUT2_PIO, BUT2_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_input(BUT3_PIO, BUT3_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);

	pio_enable_interrupt(BUT1_PIO, BUT1_IDX_MASK);
	pio_enable_interrupt(BUT2_PIO, BUT2_IDX_MASK);
	pio_enable_interrupt(BUT3_PIO, BUT3_IDX_MASK);
	pio_handler_set(BUT1_PIO, BUT1_PIO_ID, BUT1_IDX_MASK, PIO_IT_FALL_EDGE, Button1_Handler);
	pio_handler_set(BUT2_PIO, BUT2_PIO_ID, BUT2_IDX_MASK, PIO_IT_FALL_EDGE, Button2_Handler);
	pio_handler_set(BUT3_PIO, BUT3_PIO_ID, BUT3_IDX_MASK, PIO_IT_FALL_EDGE, Button3_Handler);

	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_EnableIRQ(BUT2_PIO_ID);
	NVIC_EnableIRQ(BUT3_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, 2);
	NVIC_SetPriority(BUT2_PIO_ID, 2);
	NVIC_SetPriority(BUT3_PIO_ID, 1);
};

void LED_init(int estado){
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_set_output(LED_PIO, LED_PIN_MASK, estado, 0, 0 );
	pmc_enable_periph_clk(LED2_PIO_ID);
	pio_set_output(LED2_PIO, LED2_PIN_MASK, estado, 0, 0 );
	pmc_enable_periph_clk(LED3_PIO_ID);
	pio_set_output(LED3_PIO, LED3_PIN_MASK, estado, 0, 0 );
};

void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq){
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();
	uint32_t channel = 1;

	pmc_enable_periph_clk(ID_TC);

	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, 1 | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);

	tc_start(TC, TC_CHANNEL);
}

int main(void){
	sysclk_init();

	WDT->WDT_MR = WDT_MR_WDDIS;

	LED_init(0);

	BUT_init();
	pmc_enable_periph_clk(BUZZER_PIO_ID);
	pio_set_output(BUZZER_PIO, BUZZER_PIO_IDX_MASK, 0, 0, 0);
	
	but1_flag = false;
	but2_flag = false;
	but3_flag = false;
	pio_set(LED3_PIO, LED3_PIN_MASK);
	while (1) {
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
		if (but1_flag == true) {
			play(notes, duration, 203);
			tc_stop(TC0, 0);
			but1_flag = false;
		}
		if (but2_flag == true) {
			play(melody, tempo, 78);
			tc_stop(TC0, 0);
			but2_flag = false;
		}		
	}

}

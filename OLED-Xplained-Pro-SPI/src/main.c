#include <asf.h>
#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"


// defines:
#define LED_PIO_ID_1 ID_PIOA
#define LED_PIO_1 PIOA
#define LED_IDX_1 0
#define LED_IDX_MASK_1 (1 << LED_IDX_1)

#define LED_PIO_2       PIOC
#define LED_PIO_ID_2    ID_PIOC
#define LED_IDX_2       30
#define LED_IDX_MASK_2  (1u << LED_IDX_2)

#define LED_PIO_ID_3      ID_PIOB
#define LED_PIO_3         PIOB
#define LED_IDX_3		2
#define LED_IDX_MASK_3    (1<<LED_IDX_3)

#define BUT_PIO_1 PIOD
#define BUT_PIO_ID_1 ID_PIOD
#define BUT_IDX_1 28
#define BUT_IDX_MASK_1 (1 << BUT_IDX_1)

#define BUT_PIO_2 PIOC
#define BUT_PIO_ID_2 ID_PIOC
#define BUT_IDX_2 31
#define BUT_IDX_MASK_2 (1 << BUT_IDX_2)

#define BUT_PIO_3 PIOA
#define BUT_PIO_ID_3 ID_PIOA
#define BUT_IDX_3 19
#define BUT_IDX_MASK_3 (1 << BUT_IDX_3)

//prototipos:
void Button1_Handler();
void Button2_Handler();
void Button3_Handler();
int genius_play(int seq[], int seq_len, int delay);
int user_play(int seq[], int seq_len);

volatile int f_rtt_alarme = 0;



//funcoes pra piscar o led:
void pisca_led_1(int n, int t){
	for (int i=0;i<n;i++){
		pio_set(LED_PIO_1, LED_IDX_MASK_1);
		delay_ms(t);
		pio_clear(LED_PIO_1, LED_IDX_MASK_1);
		delay_ms(t);
	}
}

void pisca_led_2(int n, int t){
	for (int i=0;i<n;i++){
		pio_set(LED_PIO_2, LED_IDX_MASK_2);
		delay_ms(t);
		pio_clear(LED_PIO_2, LED_IDX_MASK_2);
		delay_ms(t);
	}
}
void pisca_led_3(int n, int t){
	for (int i=0;i<n;i++){
		pio_set(LED_PIO_3, LED_IDX_MASK_3);
		delay_ms(t);
		pio_clear(LED_PIO_3, LED_IDX_MASK_3);
		delay_ms(t);
	}
}

//INITS:
void LED_init(int estado){
	pmc_enable_periph_clk(LED_PIO_ID_1);
	pio_set_output(LED_PIO_1, LED_IDX_MASK_1, estado, 0, 0); //3o argumento fala como incializar o led, 0 eh ligado!!!!!!!!!!!
	pmc_enable_periph_clk(LED_PIO_ID_2);
	pio_set_output(LED_PIO_2, LED_IDX_MASK_2, estado, 0, 0);
	pmc_enable_periph_clk(LED_PIO_ID_3);
	pio_set_output(LED_PIO_3, LED_IDX_MASK_3, estado, 0, 0);
};

void BUT_init(void)
{

	// Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT_PIO_ID_1);
	pmc_enable_periph_clk(BUT_PIO_ID_2);
	pmc_enable_periph_clk(BUT_PIO_ID_3);

	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pio_configure(BUT_PIO_1, PIO_INPUT, BUT_IDX_MASK_1, PIO_PULLUP);
	pio_configure(BUT_PIO_2, PIO_INPUT, BUT_IDX_MASK_2, PIO_PULLUP);
	pio_configure(BUT_PIO_3, PIO_INPUT, BUT_IDX_MASK_3, PIO_PULLUP);


	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT_PIO_1,
	BUT_PIO_ID_1,
	BUT_IDX_MASK_1,
	PIO_IT_FALL_EDGE,
	Button1_Handler);
	
	pio_handler_set(BUT_PIO_2,
	BUT_PIO_ID_2,
	BUT_IDX_MASK_2,
	PIO_IT_FALL_EDGE,
	Button2_Handler);
	
	pio_handler_set(BUT_PIO_3,
	BUT_PIO_ID_3,
	BUT_IDX_MASK_3,
	PIO_IT_FALL_EDGE,
	Button3_Handler);

	// Ativa interrupção
	pio_enable_interrupt(BUT_PIO_1, BUT_IDX_MASK_1);
	pio_enable_interrupt(BUT_PIO_2, BUT_IDX_MASK_2);
	pio_enable_interrupt(BUT_PIO_3, BUT_IDX_MASK_3);

	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT_PIO_ID_1);
	NVIC_SetPriority(BUT_PIO_ID_1, 4); // Prioridade 4
	NVIC_EnableIRQ(BUT_PIO_ID_2);
	NVIC_SetPriority(BUT_PIO_ID_2, 4);
	NVIC_EnableIRQ(BUT_PIO_ID_3);
	NVIC_SetPriority(BUT_PIO_ID_3, 4);
}


static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses)
{
	uint32_t ul_previous_time;

	/* Configure RTT for a 1 second tick interrupt */
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	ul_previous_time = rtt_read_timer_value(RTT);
	while (ul_previous_time == rtt_read_timer_value(RTT));
	
	rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);

	/* Enable RTT interrupt */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 3);
	NVIC_EnableIRQ(RTT_IRQn);
	rtt_enable_interrupt(RTT, RTT_MR_ALMIEN | RTT_MR_RTTINCIEN);
}



//HANDLERS:
volatile int but_flag_1=0;
volatile int but_flag_2=0;
volatile int but_flag_3=0;
void Button1_Handler(void){
	but_flag_1 = 1;
}
void Button2_Handler(void){
	but_flag_2 = 1;
}
void Button3_Handler(void){
	but_flag_3 = 1;
}



void RTT_Handler(void)
{
	uint32_t ul_status;

	/* Get RTT status - ACK */
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Time has changed */
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
		f_rtt_alarme = false;
		//pisca_led_2(1, 200);    // BLINK Led CIPA TENQ TAR NA MAIN QDO EH ALARMEs
	}

	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		// pin_toggle(LED_PIO, LED_IDX_MASK);    // BLINK Led
		f_rtt_alarme = true;                  // flag RTT alarme
	}
}



//Funcoes:
enum LED {LED1 = 1, LED2, LED3} LEDS;

/**
* Função para tocar a sequência
* seq[]  : Vetor contendo a sequencia (exe: seq0)
* seq_len: Tamanho da sequência (exe: seq0_len)
* delay  : Tempo em ms entre um led e outro
*
* Return 0 em caso de sucesso e 1 em caso de erro
*/

int genius_play(int seq[], int seq_len, int delay){
	for (int i=0; i<=seq_len; i++){
		if (seq[i]==LED1){
			pisca_led_1(1, delay);
		}
		if (seq[i]==LED2){
			pisca_led_2(1, delay);
		}
		if (seq[i]==LED3){
			pisca_led_3(1, delay);
		}
	}
	int x=user_play(seq,seq_len);
	if (x==1){
		return 1;
	}
	return 0;
}

/**
* Função que aguardar pelos inputs do usuário
* e verifica se sequencia está correta ou houve
* um erro.
*
* A função deve retornar:
*  0: se a sequência foi correta
*  1: se teve algum erro na sequência (retornar imediatamente)
*/
Bool game_in_session=false;
int user_play(int seq[], int seq_len){
	uint16_t pllPreScale = (int) (((float) 32768) / 4.0);
	uint32_t irqRTTvalue = 40;
//	RTT_init(pllPreScale, irqRTTvalue);
	f_rtt_alarme=false;
	but_flag_1=0;
	but_flag_2=0;
	but_flag_3=0;

	int sequser[seq_len];
	int x=0;
	while(x<=seq_len){
		if (f_rtt_alarme==true){ //estava entrando aqui toda vez mto rapido, ent meti o false em cima
			f_rtt_alarme=false;
			game_in_session=false;
			return 1;
		}
		
		
		if (but_flag_1){
			sequser[x]=LED1;
			
			
			
		}
		if (but_flag_2){
			sequser[x]=LED2;
			
			
		}
		if (but_flag_3){
			sequser[x]=LED3;
			
			
		}
		if((but_flag_1 == 1) || (but_flag_2==1) || (but_flag_3==1)){
			x++;
		//	RTT_init(pllPreScale, irqRTTvalue);
			but_flag_1=0;
			but_flag_2=0;
			but_flag_3=0;
		}
		
	}
	int iguais=1;
	for (int i=0; i<=seq_len; i++){
		if (seq[i]!=sequser[i]){
			iguais=0;
		}
	}
	if (iguais==1){
		pio_clear(LED_PIO_1, LED_IDX_MASK_1);
		pio_clear(LED_PIO_2, LED_IDX_MASK_2);
		pio_clear(LED_PIO_3, LED_IDX_MASK_3);
		delay_ms(10);
		pio_set(LED_PIO_1, LED_IDX_MASK_1);
		pio_set(LED_PIO_2, LED_IDX_MASK_2);
		pio_set(LED_PIO_3, LED_IDX_MASK_3);
		delay_ms(5000);
		pio_clear(LED_PIO_1, LED_IDX_MASK_1);
		pio_clear(LED_PIO_2, LED_IDX_MASK_2);
		pio_clear(LED_PIO_3, LED_IDX_MASK_3);
		}else{
		
		for (int i=0; i<1000; i++){
			if (but_flag_1 || but_flag_2 || but_flag_3){
				break;
				}else{
				pio_set(LED_PIO_1, LED_IDX_MASK_1);
				pio_set(LED_PIO_2, LED_IDX_MASK_2);
				pio_set(LED_PIO_3, LED_IDX_MASK_3);
				delay_ms(300);
				pio_clear(LED_PIO_1, LED_IDX_MASK_1);
				pio_clear(LED_PIO_2, LED_IDX_MASK_2);
				pio_clear(LED_PIO_3, LED_IDX_MASK_3);
			}
			
		}
	}
	game_in_session=false;
	return 0;
	
}

/**
* Função que exibe nos LEDs que o jogador acertou
* Deve manter todos os LEDs acesos por um tempo
* e então apagar
*/
void player_sucess(void){

}

/**
* Função que exibe nos LEDs que o jogador errou
* Deve piscar os LEDs até o usuário apertar um botão
*/
void player_error(void){

}


int main (void)
{
	board_init();
	sysclk_init();
	delay_init();

	// Init OLED
	gfx_mono_ssd1306_init();
	
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	f_rtt_alarme = false;
	
	/* Configura Leds */
	LED_init(0);

	/* Configura os botões */
	BUT_init();
	
	// Escreve na tela um circulo e um texto
	//gfx_mono_draw_filled_circle(20, 16, 16, GFX_PIXEL_SET, GFX_WHOLE);
	//gfx_mono_draw_string("mundo", 50,16, &sysfont);
	int seq0[] = {LED1, LED2, LED2, LED3, LED1, LED2};
	int seq0_len = sizeof(seq0)/sizeof(seq0[0])-1;
	/* Insert application code here, after the board has been initialized. */
	while(1) {
		if (game_in_session==false){
			game_in_session=true;
			genius_play(seq0,seq0_len,500);
		}
		
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
		
		
		
		
		

	}
}

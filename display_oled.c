#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include <time.h>
#include "inc/buzzer.h"

#define LED_PIN 12  // Pino do LED AZUL
#define LED_VER 11 //Pino do LED VERDE
#define PUMP_PIN 16  // Pino da bomba
#define VALVE_PIN 17  // Pino da válvula
#define BUZZER_PIN 21 // Configuração do pino do buzzer
#define BUZZER_FREQUENCY 6000
#define BUZZER_FREQUENCY_LAV 800
#define BUTTON_A 5
#define BUTTON_B 6

#define BUZZER_FREQUENCY 6000
#define BUZZER_FREQUENCY_LAV 800
#define TEMPO_FRUTAS 300000      // 5 minutos em ms
#define TEMPO_VERDURAS 600000    // 10 minutos em ms


const uint I2C_SDA = 14;
const uint I2C_SCL = 15;


// Função para exibir uma mensagem e limpar o display após um delay
void display_message(uint8_t *ssd, struct render_area *frame, const char *message, uint delay_ms) {
    memset(ssd, 0, ssd1306_buffer_length); // Limpa o buffer do display
    
    char temp_message[128];  // Buffer temporário para evitar modificar message
    strncpy(temp_message, message, sizeof(temp_message) - 1);
    temp_message[sizeof(temp_message) - 1] = '\0'; // Garante terminação

    // Divide a string nas quebras de linha
    char *line1 = temp_message;
    char *line2 = NULL;
    char *line3 = NULL;

    // Procura a primeira quebra de linha e substitui por \0
    line2 = strchr(line1, '\n');
    if (line2) {
        *line2 = '\0'; // Substitui a quebra de linha por \0 para terminar a primeira linha
        line2++;       // Avança para o início da segunda linha
    }

    // Procura a segunda quebra de linha e substitui por \0
    line3 = strchr(line2, '\n');
    if (line3) {
        *line3 = '\0'; // Substitui a quebra de linha por \0 para terminar a segunda linha
        line3++;       // Avança para o início da terceira linha
    }

    // Exibe a primeira linha
    if (line1) {
        ssd1306_draw_string(ssd, 5, 2, line1); // Primeira linha no topo
    }
    
    // Exibe a segunda linha (caso exista)
    if (line2) {
        ssd1306_draw_string(ssd, 5, 20, line2); // Segunda linha com espaçamento ajustado
    }
    
    // Exibe a terceira linha (caso exista)
    if (line3) {
        ssd1306_draw_string(ssd, 5, 38, line3); // Terceira linha com espaçamento ajustado
    }

    render_on_display(ssd, frame); // Atualiza o display
    sleep_ms(delay_ms); // Aguarda o tempo especificado
}

// Função para iniciar a lavagem
void start_wash(uint8_t *ssd, struct render_area *frame_area) {
    gpio_put(LED_PIN, 1);  // Acende o LED
    display_message(ssd, frame_area, "LAVANDO", 100);
    beep(BUZZER_PIN,BUZZER_FREQUENCY_LAV,8000);
    gpio_put(PUMP_PIN, 1);  // Liga a bomba
    gpio_put(VALVE_PIN, 1);  // Abre a válvula
    gpio_put(PUMP_PIN, 0);  // Desliga a bomba
    gpio_put(VALVE_PIN, 0);// Fecha a válvula
    gpio_put(LED_PIN, 0);  // Desliga o LED
}

// Função para exibir mensagem de "Alimentos limpos"
void show_clean_water_message(uint8_t *ssd, struct render_area *frame_area) {
    display_message(ssd, frame_area, "ALIMENTOS\nLIMPOS", 100); // Última mensagem fica fixa
    gpio_put(LED_VER, 1);// liga o led verde
    beep(BUZZER_PIN,BUZZER_FREQUENCY, 800);
    beep(BUZZER_PIN,BUZZER_FREQUENCY, 800);
    sleep_ms(2000);
}

int main() {
    stdio_init_all();   // Inicializa os tipos stdio padrão presentes ligados ao binário
    srand(time(NULL));  // Inicializa o gerador de números aleatórios

    // Inicializa o I2C
    i2c_init(i2c1, 400000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

     // Inicializa pinos GPIO
     gpio_init(PUMP_PIN);
     gpio_init(VALVE_PIN);
     gpio_init(LED_PIN);
     gpio_init(LED_VER);
     gpio_set_dir(PUMP_PIN, GPIO_OUT);
     gpio_set_dir(VALVE_PIN, GPIO_OUT);
     gpio_set_dir(LED_PIN, GPIO_OUT);
     gpio_set_dir(LED_VER, GPIO_OUT);
     gpio_put(LED_PIN, 0);
     gpio_put(LED_VER, 0);  // Inicialmente, o LED está apagado
     pwm_init_buzzer(BUZZER_PIN); // Inicializar o PWM no pino do buzzer
     gpio_init(BUTTON_A);
     gpio_set_dir(BUTTON_A, GPIO_IN);
     gpio_pull_up(BUTTON_A);
     gpio_init(BUTTON_B);
     gpio_set_dir(BUTTON_B, GPIO_IN);
     gpio_pull_up(BUTTON_B);
 

    // Inicializa o display OLED
    ssd1306_init();

    // Configura a área de renderização do display
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&frame_area);

    // Buffer do display
    uint8_t ssd[ssd1306_buffer_length];


    while (true) {
        display_message(ssd, &frame_area, "INICIAR LAVAGEM\nA-SIM B-NAO", 1000);
        
        // Espera o usuário pressionar A (Sim) ou B (Não)
        while (true) {
            if (gpio_get(BUTTON_A) == 0) {
                sleep_ms(300);
                break;  // Usuário escolheu iniciar a lavagem
            }
            if (gpio_get(BUTTON_B) == 0) {
                display_message(ssd, &frame_area, "CANCELADO", 2000);
                continue;  // Sai do loop e volta ao início
            }
        }

        display_message(ssd, &frame_area, "ESCOLHA O TIPO:\nA-FRUTAS\nB-VERDURAS", 1000);
        
        int tempo_lavagem = 0;

        // Espera o usuário escolher frutas ou verduras
        while (true) {
            if (gpio_get(BUTTON_A) == 0) {
                tempo_lavagem = TEMPO_FRUTAS;
                display_message(ssd, &frame_area, "LAVAGEM:5 MIN""\n""INICIANDO...", 2000);
                break;
            }
            if (gpio_get(BUTTON_B) == 0) {
                tempo_lavagem = TEMPO_VERDURAS;
                display_message(ssd, &frame_area, "LAVAGEM:10 MIN""\n""INICIANDO...", 2000);
                break;
            }
        }

        start_wash(ssd, &frame_area);
        show_clean_water_message(ssd, &frame_area);

        sleep_ms(5000);
    }

    return 0;
}


#include "ripes_system.h"
#include <stdlib.h> // Para rand()

// Colores para los LEDs
#define LED_ON  0x00FF00 // Verde para la serpiente
#define APPLE   0xFF0000 // Rojo para la manzana
#define EMPTY   0x000000 // Apagado

// Dimensiones de la matriz LED
#define WIDTH   35 // Ancho de la matriz
#define HEIGHT  25 // Altura de la matriz

// Direcciones
#define UP      0
#define DOWN    1
#define LEFT    2
#define RIGHT   3

// Hardware
volatile unsigned int* led_base = LED_MATRIX_0_BASE;
volatile unsigned int* d_pad_up = D_PAD_0_UP;
volatile unsigned int* d_pad_down = D_PAD_0_DOWN;
volatile unsigned int* d_pad_left = D_PAD_0_LEFT;
volatile unsigned int* d_pad_right = D_PAD_0_RIGHT;
volatile unsigned int* switch_base = SWITCHES_0_BASE;

// Variables del juego
unsigned int snake[100];    // Máximo 100 segmentos
unsigned int snake_length;  // Longitud de la serpiente
unsigned int apple_pos;     // Posición de la manzana
unsigned int direction;     // Dirección de movimiento

// Reinicia el estado del juego
void reset_game() {
    snake_length = 4;
    unsigned int start_pos = (HEIGHT / 2) * WIDTH + (WIDTH / 2);
    for (int i = 0; i < snake_length; i++) {
        snake[i] = start_pos - i * 2; // Cada segmento ocupa 2 LEDs de ancho
    }

    // Generar una manzana inicial
    do {
        apple_pos = rand() % (WIDTH * HEIGHT);
    } while (apple_pos % WIDTH + 1 >= WIDTH || apple_pos + WIDTH >= WIDTH * HEIGHT);
    direction = RIGHT;
}

// Dibuja la matriz de LEDs
void draw_matrix() {
    // Limpiar la matriz
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        led_base[i] = EMPTY;
    }

    // Dibujar la serpiente
    for (int i = 0; i < snake_length; i++) {
        unsigned int base = snake[i];
        for (int row = 0; row < 2; row++) {
            for (int col = 0; col < 2; col++) {
                if (base + row * WIDTH + col < WIDTH * HEIGHT) {
                    led_base[base + row * WIDTH + col] = LED_ON;
                }
            }
        }
    }

    // Dibujar la manzana
    for (int row = 0; row < 2; row++) {
        for (int col = 0; col < 2; col++) {
            led_base[apple_pos + row * WIDTH + col] = APPLE;
        }
    }
}

// Mueve la serpiente
void move_snake() {
    for (int i = snake_length - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }

    // Actualizar dirección
    if (*d_pad_up) direction = UP;
    if (*d_pad_down) direction = DOWN;
    if (*d_pad_left) direction = LEFT;
    if (*d_pad_right) direction = RIGHT;

    // Mover cabeza
    if (direction == UP)    snake[0] -= WIDTH * 2; // Arriba
    if (direction == DOWN)  snake[0] += WIDTH * 2; // Abajo
    if (direction == LEFT)  snake[0] -= 2;         // Izquierda
    if (direction == RIGHT) snake[0] += 2;         // Derecha
}

// Verifica colisiones
int check_collision() {
    unsigned int head = snake[0];
    unsigned int head_row = head / WIDTH;
    unsigned int head_col = head % WIDTH;

    // Colisión con bordes laterales
    if ((head_col == 0 && direction == LEFT) || 
        (head_col == WIDTH - 2 && direction == RIGHT)) {
        return 1; // Toca los bordes izquierdo o derecho
    }

    // Colisión con bordes superior o inferior
    if ((head_row == 0 && direction == UP) || 
        (head_row + 1 >= HEIGHT && direction == DOWN)) {
        return 1; // Toca los bordes superior o inferior
    }

}


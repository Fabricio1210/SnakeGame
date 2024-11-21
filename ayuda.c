#include "ripes_system.h"
#include <stdlib.h> // Para rand()

#define LED_ON  0x00FF00 // Verde para la serpiente
#define APPLE   0xFF0000 // Rojo para la manzana
#define EMPTY   0x000000 // Apagado
#define WIDTH   LED_MATRIX_0_WIDTH
#define HEIGHT  LED_MATRIX_0_HEIGHT

volatile unsigned int* led_base = LED_MATRIX_0_BASE;
volatile unsigned int* d_pad_up = D_PAD_0_UP;
volatile unsigned int* d_pad_down = D_PAD_0_DOWN;
volatile unsigned int* d_pad_left = D_PAD_0_LEFT;
volatile unsigned int* d_pad_right = D_PAD_0_RIGHT;
volatile unsigned int* switch_base = SWITCHES_0_BASE;

// Variables globales
unsigned int snake[100]; // Máximo 100 posiciones (cada segmento representado por la esquina superior izquierda)
unsigned int snake_length = 4; // Tamaño inicial
unsigned int apple_pos; // Posición de la esquina superior izquierda de la manzana
unsigned int direction = 0; // 0=arriba, 1=abajo, 2=izquierda, 3=derecha

void reset_game() {
    // Inicializar la serpiente
    snake_length = 4; // Cabeza y segmentos iniciales
    for (int i = 0; i < snake_length; i++) {
        snake[i] = (WIDTH * HEIGHT / 2) + i * 4; // Posición inicial centrada
    }
    // Generar manzana inicial en una posición válida
    do {
        apple_pos = rand() % (WIDTH * HEIGHT);
    } while (apple_pos % WIDTH + 1 >= WIDTH || apple_pos + WIDTH >= WIDTH * HEIGHT); // Asegura que sea válida
}

void draw_matrix() {
    // Limpiar la matriz
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        led_base[i] = EMPTY;
    }

    // Dibujar la serpiente (2x4 por segmento)
    for (int i = 0; i < snake_length; i++) {
        unsigned int base = snake[i];
        for (int row = 0; row < 2; row++) { // Altura = 2
            for (int col = 0; col < 4; col++) { // Anchura = 4
                led_base[base + row * WIDTH + col] = LED_ON;
            }
        }
    }

    // Dibujar la manzana (2x2)
    for (int row = 0; row < 2; row++) { // Altura = 2
        for (int col = 0; col < 2; col++) { // Anchura = 2
            led_base[apple_pos + row * WIDTH + col] = APPLE;
        }
    }
}

void move_snake() {
    // Mover el cuerpo (shift posiciones)
    for (int i = snake_length - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }

    // Cambiar la dirección de la cabeza
    if (*d_pad_up) direction = 0;
    if (*d_pad_down) direction = 1;
    if (*d_pad_left) direction = 2;
    if (*d_pad_right) direction = 3;

    // Actualizar la posición de la cabeza (2x4)
    if (direction == 0) snake[0] -= WIDTH * 2; // Arriba (dos filas)
    if (direction == 1) snake[0] += WIDTH * 2; // Abajo (dos filas)
    if (direction == 2) snake[0] -= 4;         // Izquierda (cuatro columnas)
    if (direction == 3) snake[0] += 4;         // Derecha (cuatro columnas)
}

int check_collision() {
    // Verificar bordes
    unsigned int head = snake[0];
    int head_row = head / WIDTH;
    int head_col = head % WIDTH;

    if (head_row < 0 || head_row + 1 >= HEIGHT || head_col < 0 || head_col + 3 >= WIDTH)
        return 1;

    // Verificar colisión con el cuerpo
    for (int i = 1; i < snake_length; i++) {
        unsigned int body = snake[i];
        if (head == body || head + 1 == body || head + WIDTH == body || head + WIDTH + 1 == body)
            return 1;
    }
}

void check_apple() {
    if (snake[0] == apple_pos) {
        snake_length += 4; // Crecer
        do {
            apple_pos = rand() % (WIDTH * HEIGHT);
        } while (apple_pos % WIDTH + 1 >= WIDTH || apple_pos + WIDTH >= WIDTH * HEIGHT);
    }
}

void main() {
    reset_game();
    while (1) {
        if (*switch_base & 0x01) reset_game(); // Switch 0 reinicia
        move_snake();
        if (check_collision()) reset_game(); // Reinicia al colisionar
        check_apple();
        draw_matrix();
        for (int i = 0; i < 10000; i++); // Delay
    }
}
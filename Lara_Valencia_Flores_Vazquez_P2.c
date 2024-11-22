#include "ripes_system.h"
#include <stdlib.h> // Para rand()

// Colores para los LEDs
#define LED_ON  0xFF0000 // Rojo para la serpiente
#define APPLE   0x00FF00 // Verde para la manzana
#define EMPTY   0x000000 // Apagado

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

// Dimensiones dinámicas de la matriz LED
unsigned int WIDTH;         // Ancho de la matriz
unsigned int HEIGHT;        // Alto de la matriz

// Inicializa las dimensiones de la matriz LED
void initialize_dimensions() {
    WIDTH = LED_MATRIX_0_WIDTH;
    HEIGHT = LED_MATRIX_0_HEIGHT;
}

// Reinicia el estado del juego
void reset_game() {
    snake_length = 2; // Longitud inicial de 2 segmentos
    unsigned int start_pos = (HEIGHT / 2) * WIDTH + (WIDTH / 2); // Posición inicial centrada

    // Inicializar la serpiente como un rectángulo de 2 de ancho y 2 de largo
    for (int i = 0; i < snake_length; i++) {
        snake[i] = start_pos - i * WIDTH; // Cada segmento está perfectamente alineado verticalmente
    }

    // Generar una manzana inicial
    do {
        apple_pos = rand() % (WIDTH * HEIGHT);
    } while (apple_pos % WIDTH + 1 >= WIDTH || apple_pos + WIDTH >= WIDTH * HEIGHT);
    direction = RIGHT; // Dirección inicial hacia la derecha
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
        for (int row = 0; row < 2; row++) { // Ancho de 2 LEDs por segmento
            for (int col = 0; col < 2; col++) { // Alto de 2 LEDs por segmento
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

// Verifica colisiones
int check_collision() {
    unsigned int head = snake[0];
    unsigned int head_row = head / WIDTH;
    unsigned int head_col = head % WIDTH;

    // Verificar colisión con bordes laterales (izquierdo y derecho)
    if (head_col < 0 || head_col + 1 >= WIDTH) {
        return 1; // Toca el borde izquierdo o derecho
    }

    // Verificar colisión con bordes superior e inferior
    if (head_row < 0 || head_row + 1 >= HEIGHT) {
        return 1; // Toca el borde superior o inferior
    }

    // Verificar colisión con el cuerpo
    for (int i = 1; i < snake_length; i++) {
        if (head == snake[i]) return 1;
    }
    return 0;
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

// Verifica si la serpiente come una manzana
void check_apple() {
    // Coordenadas de la cabeza de la serpiente (2x2)
    unsigned int head_x = snake[0] % WIDTH;
    unsigned int head_y = snake[0] / WIDTH;

    // Coordenadas de la manzana (2x2)
    unsigned int apple_x = apple_pos % WIDTH;
    unsigned int apple_y = apple_pos / WIDTH;

    // Verificar superposición
    if ((head_x < apple_x + 2 && head_x + 2 > apple_x) &&
        (head_y < apple_y + 2 && head_y + 2 > apple_y)) {
        // Crecer la serpiente
        unsigned int tail = snake[snake_length - 1];
        unsigned int new_segment;

        // Determinar la posición del nuevo segmento basado en la dirección del último segmento
        if (direction == UP) new_segment = tail + WIDTH * 2;
        if (direction == DOWN) new_segment = tail - WIDTH * 2;
        if (direction == LEFT) new_segment = tail + 2;
        if (direction == RIGHT) new_segment = tail - 2;

        snake[snake_length] = new_segment; // Agregar el nuevo segmento
        snake_length++; // Incrementar la longitud de la serpiente

        // Generar una nueva manzana
        do {
            apple_pos = rand() % (WIDTH * HEIGHT);
        } while (apple_pos % WIDTH + 1 >= WIDTH || apple_pos + WIDTH >= WIDTH * HEIGHT);
    }
}

// Ciclo principal del juego
void main() {
    initialize_dimensions();
    reset_game();
    while (1) {
        if (*switch_base & 0x01) reset_game();

        move_snake();
        if (check_collision()) reset_game(); // Reiniciar al detectar colisión
        check_apple();
        draw_matrix();

        for (volatile int i = 0; i < 500; i++); // Delay reducido
    }
}
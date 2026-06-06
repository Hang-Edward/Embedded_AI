#include <stdint.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

#define RCC_BASE      0x40023800UL
#define GPIOA_BASE    0x40020000UL
#define GPIOC_BASE    0x40020800UL
#define USART2_BASE   0x40004400UL

#define RCC_AHB1ENR   REG32(RCC_BASE + 0x30)
#define RCC_APB1ENR   REG32(RCC_BASE + 0x40)

#define GPIOA_MODER   REG32(GPIOA_BASE + 0x00)
#define GPIOA_OTYPER  REG32(GPIOA_BASE + 0x04)
#define GPIOA_OSPEEDR REG32(GPIOA_BASE + 0x08)
#define GPIOA_PUPDR   REG32(GPIOA_BASE + 0x0C)
#define GPIOA_ODR     REG32(GPIOA_BASE + 0x14)
#define GPIOA_AFRL    REG32(GPIOA_BASE + 0x20)

#define GPIOC_MODER   REG32(GPIOC_BASE + 0x00)
#define GPIOC_PUPDR   REG32(GPIOC_BASE + 0x0C)
#define GPIOC_IDR     REG32(GPIOC_BASE + 0x10)

#define USART2_SR     REG32(USART2_BASE + 0x00)
#define USART2_DR     REG32(USART2_BASE + 0x04)
#define USART2_BRR    REG32(USART2_BASE + 0x08)
#define USART2_CR1    REG32(USART2_BASE + 0x0C)

#define LED_PIN       5U
#define BUTTON_PIN    13U
#define RXNE          (1U << 5)
#define TXE           (1U << 7)

extern "C" uint32_t _estack;
extern "C" uint32_t _etext;
extern "C" uint32_t _sdata;
extern "C" uint32_t _edata;
extern "C" uint32_t _sbss;
extern "C" uint32_t _ebss;

extern "C" void Reset_Handler(void);
static void Default_Handler(void);

extern "C" void (* const vector_table[])(void) __attribute__((used, section(".isr_vector"))) = {
    (void (*)(void))(&_estack),
    Reset_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    0,
    0,
    0,
    0,
    Default_Handler,
    Default_Handler,
    0,
    Default_Handler,
    Default_Handler
};

static void delay(volatile uint32_t count) {
    while (count--) {
        __asm volatile ("nop");
    }
}

static void led_on(void) {
    GPIOA_ODR |= (1U << LED_PIN);
}

static void led_off(void) {
    GPIOA_ODR &= ~(1U << LED_PIN);
}

static void led_pulse(void) {
    led_on();
    delay(90000);
    led_off();
}

static void uart_putc(char c) {
    while ((USART2_SR & TXE) == 0) {
    }
    USART2_DR = (uint32_t)c;
}

static void uart_puts(const char *text) {
    while (*text) {
        uart_putc(*text++);
    }
}

static int uart_try_getc(char *out) {
    if ((USART2_SR & RXNE) == 0) {
        return 0;
    }
    *out = (char)(USART2_DR & 0xFF);
    return 1;
}

static int button_pressed(void) {
    return (GPIOC_IDR & (1U << BUTTON_PIN)) == 0U;
}

static int streq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a++ != *b++) {
            return 0;
        }
    }
    return *a == '\0' && *b == '\0';
}

static int starts_with(const char *text, const char *prefix) {
    while (*prefix) {
        if (*text++ != *prefix++) {
            return 0;
        }
    }
    return 1;
}

static void copy_text(char *dst, uint32_t dst_size, const char *src) {
    uint32_t i = 0;
    if (dst_size == 0U) {
        return;
    }
    while (src[i] && i < dst_size - 1U) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

typedef struct {
    uint8_t led_on;
    uint8_t buzzer_on;
    uint8_t vibration_on;
    char oled_text[48];
} DeviceState;

static DeviceState g_state = {
    0U,
    0U,
    0U,
    "READY"
};

static void send_status(void) {
    uart_puts("STATUS LED=");
    uart_puts(g_state.led_on ? "ON" : "OFF");
    uart_puts(";BUZZER=");
    uart_puts(g_state.buzzer_on ? "ON" : "OFF");
    uart_puts(";VIB=");
    uart_puts(g_state.vibration_on ? "ON" : "OFF");
    uart_puts(";OLED=");
    uart_puts(g_state.oled_text);
    uart_puts("\r\n");
}

static void set_led(uint8_t enabled) {
    g_state.led_on = enabled ? 1U : 0U;
    if (g_state.led_on) {
        led_on();
    } else {
        led_off();
    }
}

static void handle_command(const char *command) {
    if (streq(command, "PING")) {
        uart_puts("PONG\r\n");
        led_pulse();
    } else if (streq(command, "LED:ON") || streq(command, "LEDON")) {
        set_led(1U);
        uart_puts("OK LED ON\r\n");
    } else if (streq(command, "LED:OFF") || streq(command, "LEDOFF")) {
        set_led(0U);
        uart_puts("OK LED OFF\r\n");
    } else if (streq(command, "BUZZER:ON")) {
        g_state.buzzer_on = 1U;
        uart_puts("OK BUZZER ON\r\n");
    } else if (streq(command, "BUZZER:OFF")) {
        g_state.buzzer_on = 0U;
        uart_puts("OK BUZZER OFF\r\n");
    } else if (streq(command, "VIB:ON")) {
        g_state.vibration_on = 1U;
        uart_puts("OK VIB ON\r\n");
    } else if (streq(command, "VIB:OFF")) {
        g_state.vibration_on = 0U;
        uart_puts("OK VIB OFF\r\n");
    } else if (starts_with(command, "OLED:TEXT=")) {
        copy_text(g_state.oled_text, sizeof(g_state.oled_text), command + 10);
        uart_puts("OK OLED TEXT\r\n");
    } else if (streq(command, "STATUS?")) {
        send_status();
    } else {
        uart_puts("ERR UNKNOWN COMMAND\r\n");
    }
}

static void clock_gpio_uart_init(void) {
    /* NUCLEO-F446RE reset default uses 16MHz HSI; keep the clock setup simple. */
    RCC_AHB1ENR |= (1U << 0);   /* GPIOA clock */
    RCC_AHB1ENR |= (1U << 2);   /* GPIOC clock */
    RCC_APB1ENR |= (1U << 17);  /* USART2 clock */

    /* PA5 = board LED LD2, output mode. */
    GPIOA_MODER &= ~(3U << (LED_PIN * 2U));
    GPIOA_MODER |=  (1U << (LED_PIN * 2U));

    /* PC13 = blue user button B1, input with pull-up. Pressed reads low. */
    GPIOC_MODER &= ~(3U << (BUTTON_PIN * 2U));
    GPIOC_PUPDR &= ~(3U << (BUTTON_PIN * 2U));
    GPIOC_PUPDR |=  (1U << (BUTTON_PIN * 2U));

    /* PA2 = USART2 TX, PA3 = USART2 RX, alternate function AF7. */
    GPIOA_MODER &= ~((3U << (2U * 2U)) | (3U << (3U * 2U)));
    GPIOA_MODER |=  ((2U << (2U * 2U)) | (2U << (3U * 2U)));
    GPIOA_AFRL &= ~((0xFU << (2U * 4U)) | (0xFU << (3U * 4U)));
    GPIOA_AFRL |=  ((7U << (2U * 4U)) | (7U << (3U * 4U)));
    GPIOA_OSPEEDR |= (3U << (2U * 2U)) | (3U << (3U * 2U));
    GPIOA_PUPDR &= ~((3U << (2U * 2U)) | (3U << (3U * 2U)));
    GPIOA_PUPDR |=  (1U << (3U * 2U)); /* RX pull-up */

    /* USART2 uses 115200 8N1. HSI 16MHz, oversampling by 16, BRR ~= 139. */
    USART2_CR1 = 0;
    USART2_BRR = 139U;
    USART2_CR1 = (1U << 13) | (1U << 3) | (1U << 2); /* UE, TE, RE */
}

static void poll_uart_command(char *buffer, uint32_t *len) {
    char c = 0;
    if (!uart_try_getc(&c)) {
        return;
    }

    if (c == '\r' || c == '\n') {
        if (*len > 0U) {
            buffer[*len] = '\0';
            handle_command(buffer);
            *len = 0U;
        }
    } else if (*len < 79U) {
        buffer[(*len)++] = c;
    } else {
        *len = 0U;
        uart_puts("ERR COMMAND TOO LONG\r\n");
    }
}

static void poll_button_event(uint8_t *last_button, uint8_t *button_lock) {
    const uint8_t now_pressed = button_pressed() ? 1U : 0U;
    if (now_pressed && !(*last_button) && !(*button_lock)) {
        delay(70000);
        if (button_pressed()) {
            uart_puts("EVENT BUTTON PRESSED\r\n");
            led_pulse();
            *button_lock = 1U;
        }
    }

    if (!now_pressed) {
        *button_lock = 0U;
    }
    *last_button = now_pressed;
}

static void app_main(void) {
    char buffer[80];
    uint32_t len = 0U;
    uint8_t last_button = 0U;
    uint8_t button_lock = 0U;

    clock_gpio_uart_init();
    led_pulse();
    uart_puts("\r\nNUCLEO-F446RE AI BRIDGE READY\r\n");
    uart_puts("Commands: PING, LED:ON, LED:OFF, BUZZER:ON, VIB:ON, OLED:TEXT=..., STATUS?\r\n");
    uart_puts("Events: EVENT BUTTON PRESSED\r\n");

    while (1) {
        poll_uart_command(buffer, &len);
        poll_button_event(&last_button, &button_lock);
    }
}

extern "C" void Reset_Handler(void) {
    uint32_t *src = &_etext;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0;
    }

    app_main();
}

static void Default_Handler(void) {
    while (1) {
    }
}

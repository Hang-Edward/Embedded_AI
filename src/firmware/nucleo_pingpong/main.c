#include <stdint.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

#define RCC_BASE      0x40023800UL
#define GPIOA_BASE    0x40020000UL
#define USART2_BASE   0x40004400UL

#define RCC_AHB1ENR   REG32(RCC_BASE + 0x30)
#define RCC_APB1ENR   REG32(RCC_BASE + 0x40)

#define GPIOA_MODER   REG32(GPIOA_BASE + 0x00)
#define GPIOA_OTYPER  REG32(GPIOA_BASE + 0x04)
#define GPIOA_OSPEEDR REG32(GPIOA_BASE + 0x08)
#define GPIOA_PUPDR   REG32(GPIOA_BASE + 0x0C)
#define GPIOA_ODR     REG32(GPIOA_BASE + 0x14)
#define GPIOA_AFRL    REG32(GPIOA_BASE + 0x20)

#define USART2_SR     REG32(USART2_BASE + 0x00)
#define USART2_DR     REG32(USART2_BASE + 0x04)
#define USART2_BRR    REG32(USART2_BASE + 0x08)
#define USART2_CR1    REG32(USART2_BASE + 0x0C)

#define LED_PIN       5U
#define RXNE          (1U << 5)
#define TXE           (1U << 7)

extern uint32_t _estack;
extern uint32_t _etext;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

void Reset_Handler(void);
static void Default_Handler(void);

__attribute__((section(".isr_vector")))
void (* const vector_table[])(void) = {
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

static char uart_getc_blocking(void) {
    while ((USART2_SR & RXNE) == 0) {
    }
    return (char)(USART2_DR & 0xFF);
}

static int streq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a++ != *b++) {
            return 0;
        }
    }
    return *a == '\0' && *b == '\0';
}

static void clock_gpio_uart_init(void) {
    /* 中文注释：NUCLEO-F446RE 复位后默认使用 16MHz HSI，这里保持默认时钟以降低复杂度。 */
    RCC_AHB1ENR |= (1U << 0);   /* GPIOA clock */
    RCC_APB1ENR |= (1U << 17);  /* USART2 clock */

    /* PA5 = board LED LD2, output mode. */
    GPIOA_MODER &= ~(3U << (LED_PIN * 2U));
    GPIOA_MODER |=  (1U << (LED_PIN * 2U));

    /* PA2 = USART2 TX, PA3 = USART2 RX, alternate function AF7. */
    GPIOA_MODER &= ~((3U << (2U * 2U)) | (3U << (3U * 2U)));
    GPIOA_MODER |=  ((2U << (2U * 2U)) | (2U << (3U * 2U)));
    GPIOA_AFRL &= ~((0xFU << (2U * 4U)) | (0xFU << (3U * 4U)));
    GPIOA_AFRL |=  ((7U << (2U * 4U)) | (7U << (3U * 4U)));
    GPIOA_OSPEEDR |= (3U << (2U * 2U)) | (3U << (3U * 2U));
    GPIOA_PUPDR &= ~((3U << (2U * 2U)) | (3U << (3U * 2U)));
    GPIOA_PUPDR |=  (1U << (3U * 2U)); /* RX pull-up */

    /*
     * 中文注释：USART2 使用 115200 8N1。
     * HSI 16MHz, oversampling by 16, BRR ~= 16000000 / 115200 = 139.
     */
    USART2_CR1 = 0;
    USART2_BRR = 139U;
    USART2_CR1 = (1U << 13) | (1U << 3) | (1U << 2); /* UE, TE, RE */
}

static void app_main(void) {
    char buffer[32];
    uint32_t len = 0;

    clock_gpio_uart_init();
    led_pulse();
    uart_puts("\r\nNUCLEO-F446RE PING/PONG READY\r\n");
    uart_puts("Send PING and press Enter.\r\n");

    while (1) {
        char c = uart_getc_blocking();
        if (c == '\r' || c == '\n') {
            if (len > 0) {
                buffer[len] = '\0';
                if (streq(buffer, "PING")) {
                    uart_puts("PONG\r\n");
                    led_pulse();
                } else if (streq(buffer, "LEDON")) {
                    led_on();
                    uart_puts("OK LED ON\r\n");
                } else if (streq(buffer, "LEDOFF")) {
                    led_off();
                    uart_puts("OK LED OFF\r\n");
                } else {
                    uart_puts("ERR UNKNOWN COMMAND\r\n");
                }
                len = 0;
            }
        } else if (len < sizeof(buffer) - 1U) {
            buffer[len++] = c;
        } else {
            len = 0;
            uart_puts("ERR COMMAND TOO LONG\r\n");
        }
    }
}

void Reset_Handler(void) {
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

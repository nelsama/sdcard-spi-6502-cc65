/**
 * SDCARD.C - Librería SD Card para 6502
 * Loops de 512 bytes optimizados en ASM (sdcard_asm.s)
 */

#include <stdint.h>
#include "sdcard.h"
#include "spi.h"

/* Funciones ASM optimizadas para transferencia de bloques */
extern void sd_read_block(uint8_t *buf);
extern void sd_write_block(const uint8_t *buf);

/* Variables de estado */
static uint8_t sd_type_var = 0;   /* 0=none, 1=SD, 2=SDHC */
static uint8_t sd_ready_var = 0;

/* Comandos SD */
#define CMD0   0x40   /* GO_IDLE_STATE */
#define CMD8   0x48   /* SEND_IF_COND */
#define CMD17  0x51   /* READ_SINGLE_BLOCK */
#define CMD24  0x58   /* WRITE_BLOCK */
#define CMD55  0x77   /* APP_CMD */
#define CMD58  0x7A   /* READ_OCR */
#define ACMD41 0x69   /* SD_SEND_OP_COND */

/* ============================================================================
 * Funciones internas
 * ============================================================================ */

static void cs_low(void) {
    spi_select(0x01);
}

static void cs_high(void) {
    spi_deselect();
}

static uint8_t sd_cmd(uint8_t cmd, uint32_t arg, uint8_t crc) {
    uint8_t r, i;
    
    spi_transfer(0xFF);
    spi_transfer(cmd);
    spi_transfer((arg >> 24) & 0xFF);
    spi_transfer((arg >> 16) & 0xFF);
    spi_transfer((arg >> 8) & 0xFF);
    spi_transfer(arg & 0xFF);
    spi_transfer(crc);
    
    /* Esperar respuesta (no 0xFF) */
    for (i = 0; i < 10; i++) {
        r = spi_transfer(0xFF);
        if (r != 0xFF) return r;
    }
    return 0xFF;
}

static uint8_t sd_acmd(uint8_t cmd, uint32_t arg) {
    sd_cmd(CMD55, 0, 0x65);
    return sd_cmd(cmd, arg, 0xFF);
}

/* ============================================================================
 * Funciones públicas
 * ============================================================================ */

uint8_t sd_init(void) {
    uint8_t r, i;
    uint16_t retry;
    
    sd_ready_var = 0;
    sd_type_var = 0;
    
    /* Inicializar SPI */
    spi_init();
    
    /* 1. Power-up: 80+ clocks con CS alto */
    cs_high();
    for (i = 0; i < 10; i++) {
        spi_transfer(0xFF);
    }
    
    /* 2. CMD0: GO_IDLE_STATE */
    cs_low();
    r = sd_cmd(CMD0, 0, 0x95);
    cs_high();
    spi_transfer(0xFF);
    
    if (r != 0x01) {
        return SD_ERROR_INIT;
    }
    
    /* 3. CMD8: SEND_IF_COND (detectar SDHC) */
    cs_low();
    r = sd_cmd(CMD8, 0x000001AA, 0x87);
    if (r == 0x01) {
        /* SDHC: leer 4 bytes de respuesta */
        spi_transfer(0xFF);
        spi_transfer(0xFF);
        spi_transfer(0xFF);
        spi_transfer(0xFF);
        sd_type_var = 2;  /* SDv2 (posible SDHC) */
    } else {
        sd_type_var = 1;  /* SDv1 */
    }
    cs_high();
    spi_transfer(0xFF);
    
    /* 4. ACMD41: SD_SEND_OP_COND (inicializar) */
    for (retry = 0; retry < 1000; retry++) {
        cs_low();
        r = sd_acmd(ACMD41, (sd_type_var == 2) ? 0x40000000 : 0);
        cs_high();
        spi_transfer(0xFF);
        
        if (r == 0x00) break;
    }
    if (r != 0x00) {
        return SD_ERROR_TIMEOUT;
    }
    
    /* 5. CMD58: READ_OCR (verificar CCS para SDHC) */
    if (sd_type_var == 2) {
        cs_low();
        r = sd_cmd(CMD58, 0, 0xFF);
        if (r == 0x00) {
            uint8_t ocr0 = spi_transfer(0xFF);
            spi_transfer(0xFF);
            spi_transfer(0xFF);
            spi_transfer(0xFF);
            if (!(ocr0 & 0x40)) {
                sd_type_var = 1;  /* CCS=0, no es SDHC */
            }
        }
        cs_high();
        spi_transfer(0xFF);
    }
    
    sd_ready_var = 1;
    return SD_OK;
}

uint8_t sd_read_sector(uint32_t sector, uint8_t *buf) {
    uint8_t r;
    uint16_t i;
    
    /* Si no es SDHC, convertir sector a dirección byte */
    if (sd_type_var != 2) {
        sector <<= 9;  /* *512 */
    }
    
    cs_low();
    r = sd_cmd(CMD17, sector, 0xFF);
    
    if (r != 0x00) {
        cs_high();
        return SD_ERROR_CMD;
    }
    
    /* Esperar token de datos (0xFE) */
    for (i = 0; i < 5000; i++) {
        r = spi_transfer(0xFF);
        if (r == 0xFE) break;
    }
    if (r != 0xFE) {
        cs_high();
        return SD_ERROR_TIMEOUT;
    }
    
    /* Leer 512 bytes (ASM optimizado) */
    sd_read_block(buf);
    
    /* Descartar CRC (2 bytes) */
    spi_transfer(0xFF);
    spi_transfer(0xFF);
    
    cs_high();
    spi_transfer(0xFF);
    
    return SD_OK;
}

uint8_t sd_write_sector(uint32_t sector, const uint8_t *buf) {
    uint8_t r;
    uint16_t i;
    
    /* Si no es SDHC, convertir sector a dirección byte */
    if (sd_type_var != 2) {
        sector <<= 9;
    }
    
    cs_low();
    r = sd_cmd(CMD24, sector, 0xFF);
    
    if (r != 0x00) {
        cs_high();
        return SD_ERROR_CMD;
    }
    
    spi_transfer(0xFF);   /* Dummy byte */
    spi_transfer(0xFE);   /* Data token */
    
    /* Escribir 512 bytes (ASM optimizado) */
    sd_write_block(buf);
    
    /* CRC dummy (2 bytes) */
    spi_transfer(0xFF);
    spi_transfer(0xFF);
    
    /* Leer respuesta */
    r = spi_transfer(0xFF);
    if ((r & 0x1F) != 0x05) {
        cs_high();
        return SD_ERROR_WRITE;
    }
    
    /* Esperar que termine la escritura (busy = 0x00) */
    for (i = 0; i < 10000; i++) {
        if (spi_transfer(0xFF) != 0x00) break;
    }
    
    cs_high();
    spi_transfer(0xFF);
    
    return SD_OK;
}

uint8_t sd_is_ready(void) {
    return sd_ready_var;
}

uint8_t sd_get_type(void) {
    return sd_type_var;
}

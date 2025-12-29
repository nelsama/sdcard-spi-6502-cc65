/**
 * SDCARD.H - Librería SD Card simplificada para 6502
 * 
 * Proporciona acceso básico a tarjetas SD/SDHC en modo SPI.
 * Soporta lectura y escritura de sectores de 512 bytes.
 * 
 * Uso típico:
 *   sd_init();
 *   sd_read_sector(0, buffer);
 *   sd_write_sector(0, buffer);
 */

#ifndef SDCARD_H
#define SDCARD_H

#include <stdint.h>

/* ============================================================================
 * CÓDIGOS DE ERROR
 * ============================================================================ */

#define SD_OK               0x00    /* Operación exitosa */
#define SD_ERROR_TIMEOUT    0x01    /* Timeout esperando respuesta */
#define SD_ERROR_CMD        0x02    /* Error en comando */
#define SD_ERROR_INIT       0x03    /* Error de inicialización */
#define SD_ERROR_READ       0x04    /* Error de lectura */
#define SD_ERROR_WRITE      0x05    /* Error de escritura */
#define SD_ERROR_CRC        0x06    /* Error de CRC */
#define SD_ERROR_NO_CARD    0x07    /* No hay tarjeta */

/* ============================================================================
 * TIPOS
 * ============================================================================ */

/* Tipo para número de sector (32 bits para SDHC) */
typedef uint32_t sector_t;

/* ============================================================================
 * FUNCIONES PRINCIPALES
 * ============================================================================ */

/**
 * Inicializa la tarjeta SD
 * @return SD_OK si éxito, código de error si falla
 */
uint8_t sd_init(void);

/**
 * Lee un sector de 512 bytes
 * @param sector Número de sector (LBA)
 * @param buffer Puntero al buffer de 512 bytes
 * @return SD_OK si éxito, código de error si falla
 */
uint8_t sd_read_sector(sector_t sector, uint8_t *buffer);

/**
 * Escribe un sector de 512 bytes
 * @param sector Número de sector (LBA)
 * @param buffer Puntero al buffer de 512 bytes
 * @return SD_OK si éxito, código de error si falla
 */
uint8_t sd_write_sector(sector_t sector, const uint8_t *buffer);

/* ============================================================================
 * FUNCIONES AUXILIARES
 * ============================================================================ */

/**
 * Verifica si hay tarjeta presente y lista
 * @return 1 si lista, 0 si no
 */
uint8_t sd_is_ready(void);

/**
 * Obtiene el tipo de tarjeta
 * @return 1=SD, 2=SDHC, 0=desconocido/no inicializada
 */
uint8_t sd_get_type(void);

/**
 * Lee múltiples sectores consecutivos
 * @param sector Sector inicial
 * @param buffer Buffer destino
 * @param count Número de sectores a leer
 * @return SD_OK si éxito, código de error si falla
 */
uint8_t sd_read_sectors(sector_t sector, uint8_t *buffer, uint8_t count);

/**
 * Escribe múltiples sectores consecutivos
 * @param sector Sector inicial
 * @param buffer Buffer origen
 * @param count Número de sectores a escribir
 * @return SD_OK si éxito, código de error si falla
 */
uint8_t sd_write_sectors(sector_t sector, const uint8_t *buffer, uint8_t count);

#endif /* SDCARD_H */

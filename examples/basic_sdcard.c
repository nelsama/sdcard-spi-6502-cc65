/**
 * Ejemplo básico de uso de la librería SD Card
 * Lectura y escritura de sectores
 */

#include <stdint.h>
#include "sdcard.h"

/* Buffer para un sector */
uint8_t buffer[512];

int main(void) {
    uint8_t result;
    uint16_t i;
    
    /* Inicializar tarjeta SD */
    result = sd_init();
    if (result != SD_OK) {
        /* Error de inicialización */
        while (1);
    }
    
    /* Verificar tipo de tarjeta */
    if (sd_get_type() == 2) {
        /* Es SDHC */
    }
    
    /* Leer sector 0 (MBR) */
    result = sd_read_sector(0, buffer);
    if (result == SD_OK) {
        /* Verificar firma MBR (0x55, 0xAA en bytes 510-511) */
        if (buffer[510] == 0x55 && buffer[511] == 0xAA) {
            /* MBR válido */
        }
    }
    
    /* Escribir datos en sector 100 */
    for (i = 0; i < 512; i++) {
        buffer[i] = (uint8_t)i;
    }
    result = sd_write_sector(100, buffer);
    
    /* Verificar escritura leyendo el sector */
    result = sd_read_sector(100, buffer);
    
    while (1);
    return 0;
}

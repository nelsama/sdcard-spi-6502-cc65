# sdcard-spi-6502-cc65

Librería para acceso a tarjetas SD/SDHC via SPI para sistemas 6502 con cc65.

## Características

- Soporta SD (v1), SD (v2) y SDHC
- Sectores de 512 bytes
- Lógica en C, loops optimizados en ASM
- Tamaño: ~1.4 KB

## Instalación

```bash
git clone https://github.com/nelsama/sdcard-spi-6502-cc65.git libs/sdcard-6502-cc65
```

## Dependencias

Esta librería requiere:
- [spi-6502-cc65](https://github.com/nelsama/spi-6502-cc65)

## API

```c
#include "sdcard.h"

// Inicialización
uint8_t sd_init(void);

// Lectura/Escritura de sectores (512 bytes)
uint8_t sd_read_sector(uint32_t sector, uint8_t *buffer);
uint8_t sd_write_sector(uint32_t sector, const uint8_t *buffer);

// Estado
uint8_t sd_is_ready(void);    // 1 si lista
uint8_t sd_get_type(void);    // 1=SD, 2=SDHC
```

## Códigos de Error

```c
SD_OK             0   // Éxito
SD_ERROR_TIMEOUT  1   // Timeout
SD_ERROR_CMD      2   // Error de comando
SD_ERROR_INIT     3   // Error inicialización
SD_ERROR_READ     4   // Error de lectura
SD_ERROR_WRITE    5   // Error de escritura
```

## Ejemplo

```c
#include "sdcard.h"

uint8_t buffer[512];

int main(void) {
    if (sd_init() != SD_OK) {
        return 1;
    }
    
    // Leer sector 0 (MBR)
    sd_read_sector(0, buffer);
    
    // Escribir sector 100
    buffer[0] = 0x55;
    sd_write_sector(100, buffer);
    
    return 0;
}
```

## Direccionamiento

| Tipo | Método |
|------|--------|
| SD (v1/v2) | Bytes (sector × 512) - automático |
| SDHC | Bloques (sector directo) - automático |

La librería detecta el tipo y ajusta automáticamente.

## Integración con Makefile

```makefile
SDCARD_DIR = libs/sdcard-6502-cc65
INCLUDES += -I$(SDCARD_DIR)

$(BUILD_DIR)/sdcard.o: $(SDCARD_DIR)/sdcard.c
	$(CL65) -t none $(INCLUDES) -c -o $@ $<

$(BUILD_DIR)/sdcard_asm.o: $(SDCARD_DIR)/sdcard_asm.s
	$(CA65) -t none -o $@ $<
```

## Archivos

| Archivo | Descripción |
|---------|-------------|
| `sdcard.h` | Header con API |
| `sdcard.c` | Lógica de comandos SD |
| `sdcard_asm.s` | Loops de 512 bytes optimizados |
| `examples/` | Ejemplos de uso |

## Arquitectura

```
sdcard.c
├── sd_init()         → Secuencia de inicialización
├── sd_read_sector()  → CMD17 + sd_read_block (ASM)
└── sd_write_sector() → CMD24 + sd_write_block (ASM)

sdcard_asm.s
├── sd_read_block()   → Lee 512 bytes (optimizado)
└── sd_write_block()  → Escribe 512 bytes (optimizado)
```

## Licencia

MIT License - ver [LICENSE](LICENSE)

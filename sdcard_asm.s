;-----------------------------------------------------------------------------
; SDCARD_ASM.S - Funciones optimizadas para transferencia SD
; Loops de 512 bytes en ASM para máxima velocidad
;-----------------------------------------------------------------------------

        .export _sd_read_block
        .export _sd_write_block

        .importzp ptr1

;-----------------------------------------------------------------------------
; Constantes SPI (deben coincidir con spi.s)
;-----------------------------------------------------------------------------
SPI_RX_DATA     = $C040
SPI_TX_DATA     = $C041
SPI_STATUS      = $C042
SPI_STAT_TRDY   = $20       ; Bit 5: TX Ready
SPI_STAT_RRDY   = $40       ; Bit 6: RX Ready

        .code

;-----------------------------------------------------------------------------
; void sd_read_block(uint8_t *buf)
; Lee 512 bytes desde SPI al buffer (envía 0xFF)
; Entrada: A/X = puntero al buffer (A=low, X=high)
;-----------------------------------------------------------------------------
.proc _sd_read_block
        sta     ptr1            ; Buffer low
        stx     ptr1+1          ; Buffer high
        
        ldx     #2              ; 2 páginas de 256 = 512 bytes
        ldy     #0              ; Índice dentro de página

@page_loop:
@byte_loop:
        ; Esperar TX ready
@wait_tx:
        lda     SPI_STATUS
        and     #SPI_STAT_TRDY
        beq     @wait_tx
        
        lda     #$FF
        sta     SPI_TX_DATA     ; Enviar dummy byte
        
        ; Esperar RX ready
@wait_rx:
        lda     SPI_STATUS
        and     #SPI_STAT_RRDY
        beq     @wait_rx
        
        lda     SPI_RX_DATA     ; Leer dato
        sta     (ptr1),y        ; Guardar en buffer
        iny
        bne     @byte_loop      ; Siguiente byte (256 iteraciones)
        
        ; Siguiente página
        inc     ptr1+1
        dex
        bne     @page_loop      ; Segunda página
        
        rts
.endproc

;-----------------------------------------------------------------------------
; void sd_write_block(const uint8_t *buf)
; Escribe 512 bytes desde buffer a SPI
; Entrada: A/X = puntero al buffer (A=low, X=high)
;-----------------------------------------------------------------------------
.proc _sd_write_block
        sta     ptr1            ; Buffer low
        stx     ptr1+1          ; Buffer high
        
        ldx     #2              ; 2 páginas de 256 = 512 bytes
        ldy     #0              ; Índice dentro de página

@page_loop:
@byte_loop:
        ; Esperar TX ready
@wait_tx:
        lda     SPI_STATUS
        and     #SPI_STAT_TRDY
        beq     @wait_tx
        
        lda     (ptr1),y        ; Leer del buffer
        sta     SPI_TX_DATA     ; Enviar
        
        ; Esperar que termine (leer y descartar)
@wait_rx:
        lda     SPI_STATUS
        and     #SPI_STAT_RRDY
        beq     @wait_rx
        
        lda     SPI_RX_DATA     ; Descartar dato recibido
        iny
        bne     @byte_loop      ; Siguiente byte (256 iteraciones)
        
        ; Siguiente página
        inc     ptr1+1
        dex
        bne     @page_loop      ; Segunda página
        
        rts
.endproc

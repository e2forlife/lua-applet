/*
 * ymodem.h
 *
 *  Created on: Oct 10, 2022
 *      Author: CErhardt
 */

#ifndef DRIVERS_YMODEM_H_
#define DRIVERS_YMODEM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Types ------------------------------------------------------------------ */

/* Definitions and constants ============================================== */
#define YM_RES_OK               (0)
#define YM_RES_TIMEOUT          (1)
#define YM_RES_END_OF_TRANSFER  (2)
#define YM_RES_CANCEL           (3)
#define YM_RES_ERROR            (4)
#define YM_RES_ERR_CRC          (5)
#define YM_RES_BUSY             (6)
#define YM_RES_ACK              (7)

/* Public API ------------------------------------------------------------- */

uint32_t ymodem_receive_xmodem_unsafe(bool ascii_mode);
uint32_t ymodem_send_xmodem_unsafe( void );
uint32_t ymodem_get_error(void);

/* Callback API =========================================================== */
/* these functions are typically overridden by the application to defer
 * operation to the device of choosing when assigning the API for the
 * YMODEM Interface.  This protocol has standard serial APIs plus a stream
 * read/write API for loading/storing the file data from the transfer.
 */

uint32_t ym_stream_write(uint32_t block_id, uint8_t *block, uint32_t size);
uint32_t ym_stream_read(uint8_t *block, uint32_t size, bool first, uint32_t *br);
void ym_flush( void );
void ym_putc(char c);
uint32_t ym_getc( uint32_t timeout);
void ym_send( uint8_t* packet, uint32_t size);
uint32_t ym_receive(uint8_t* packet, uint32_t size, uint32_t timeout);
/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif


#endif /* DRIVERS_YMODEM_H_ */

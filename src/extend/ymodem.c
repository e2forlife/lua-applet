/*
 * ymodem.c
 *
 *  Created on: Oct 10, 2022
 *      Author: CErhardt
 */
/*
 * Phobos (c) 2016-2021 by E2ForLife.com
 *
 * Phobos is licensed under a
 * Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *
 * You should have received a copy of the license along with this
 * work. If not, see <http://creativecommons.org/licenses/by-nc-sa/4.0/>.
 */
/* ------------------------------------------------------------------------ */
#include "ymodem.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
/* ------------------------------------------------------------------------ */
#define XOPT_YMODEM_MAX_PACKET_SIZE   2048
#define XOPT_YMODEM_TIMEOUT_CHAR      1000
#define XOPT_YMODEM_MAX_TRIES         10

/* ------------------------------------------------------------------------ */
#define _ym_max(a,b)   ( (a>b)?a:b)
#define _ym_min(a,b)   ( (a<b)?a:b)

/* constants defined by YModem protocol */
#define YM_SOH                     (0x01)  /* start of 128-byte data packet */
#define YM_STX                     (0x02)  /* start of 1024-byte data packet */
#define YM_EOT                     (0x04)  /* End Of Transmission */
#define YM_ACK                     (0x06)  /* ACKnowledge, receive OK */
#define YM_NAK                     (0x15)  /* Negative ACKnowledge, receiver ERROR, retry */
#define YM_CAN                     (0x18)  /* two CAN in succession will abort transfer */
#define YM_CRC                     (0x43)  /* 'C' == 0x43, request 16-bit CRC, use in place of first NAK for CRC mode */
#define YM_ABT1                    (0x41)  /* 'A' == 0x41, assume try abort by user typing */
#define YM_ABT2                    (0x61)  /* 'a' == 0x61, assume try abort by user typing */
#define YM_EOF                     (0x1A)  /* CTRL-Z terminator */

/* Definitions for helper functions ======================================= */
/* _YM_DATASIZE() is a helper macro for determining the packet length
 * from the packet starting character.  XMODEM-1K uses STX, XMODEM uses SOH
 */
#define _YM_DATASIZE(b)            ( (b[0]==YM_STX)?1024:((b[0]==YM_SOH)?128:0))

/*
 * These are defined configuration macros for binding the YMODEM driver
 * implementation to the underlying tick API for delays and timing.
 */
#define _YM_DELAY                  Sleep

/* Private Data =========================================================== */
static uint32_t _ym_fs_state = YM_RES_OK;
static uint8_t _ym_packet_ws[ XOPT_YMODEM_MAX_PACKET_SIZE ];

/* Interface APIs --------------------------------------------------------- */

/* Private APIs =========================================================== */
/* ------------------------------------------------------------------------ */
static uint32_t ym_calc_crc16(uint8_t *packet, uint32_t len, uint32_t crc)
{
    int i;
    uint32_t comp_crc = crc;

    for (; len > 0; len--) { /* Step through bytes in memory */

        comp_crc = comp_crc ^ (*packet++ << 8); /* Fetch byte from memory, XOR into CRC top byte*/
        for (i = 0; i < 8; i++) { /* Prepare to rotate 8 bits */
            comp_crc = comp_crc << 1; /* rotate */
            if (comp_crc & 0x10000) /* bit 15 was set (now bit 16)... */
            comp_crc = (comp_crc ^ 0x1021) & 0xFFFF; /* XOR with XMODEM polynomic */
            /* and ensure CRC remains 16-bit value */
        } /* Loop for 8 bits */
    } /* Loop until num=0 */
    return comp_crc;
}

/* ------------------------------------------------ */
/**
 * @fn uint32_t ymodem_receive_xmodem_packet(uint32_t *port, uint8_t *buf, bool crc)
 * @brief receive a packet formatted for XMODEM
 * @param port pointer to the serial device
 * @param buf pointer to the packet buffer
 * @param crc bool identifying CRC packet mode
 * @retval res_timeout A timeout has occured
 * @retval res_busy End of transfer detected
 * @retval res_cancel Cancel has been requested
 * @retval P_RES_OK packet receive correctly
 *
 */
static uint32_t ymodem_receive_xmodem_packet( bool crc )
{
    uint32_t bytes = 0;
    /* receive the first packet character */
    uint32_t sop = ym_getc( XOPT_YMODEM_TIMEOUT_CHAR );
    if (sop == 0xFFFFFFFF) return YM_RES_TIMEOUT;
    _ym_packet_ws[0] = (uint8_t)sop;

    /* determine the length of the packet */
    uint32_t plen = 0;
    if (sop == YM_SOH) plen = 128;
    else if (sop == YM_STX) plen = 1024;
    else if (sop == YM_EOT) return YM_RES_END_OF_TRANSFER;
    else if (sop == YM_CAN) return YM_RES_CANCEL;
    else return YM_RES_ERROR;

    /* read the block ID */
    bytes = ym_receive(&_ym_packet_ws[1], 2, XOPT_YMODEM_TIMEOUT_CHAR);
    if (bytes <2) return YM_RES_TIMEOUT;
    /* receive data payload */
    bytes = ym_receive(&_ym_packet_ws[3], plen, XOPT_YMODEM_TIMEOUT_CHAR);
    if (bytes < plen) return YM_RES_TIMEOUT;

    /* receive CRC and/or checksum */
    if (crc) bytes = ym_receive(&_ym_packet_ws[3 + plen], 2, XOPT_YMODEM_TIMEOUT_CHAR);
    else bytes = ym_receive(&_ym_packet_ws[3 + plen], 1, XOPT_YMODEM_TIMEOUT_CHAR);

    return (bytes<((crc)?2:1))?YM_RES_TIMEOUT:YM_RES_OK;
}
/* ------------------------------------------------------------------------ */
static uint32_t ymodem_verify_xmodem_packet(uint8_t *packet, bool crc)
{
	union {
		uint32_t u32;
		uint16_t u16[2];
		uint8_t  u8[4];
	} check;

    if (crc) {
        uint16_t *verify = (uint16_t*) (packet + _YM_DATASIZE(packet) + 3);
        check.u32 = ym_calc_crc16(&packet[3], _YM_DATASIZE(packet), 0);
        check.u16[0] = ((check.u16[0] >> 8) & 0xFF) | (check.u16[0] << 8);
        return (check.u16[0] == *verify) ? YM_RES_OK : YM_RES_ERR_CRC;
    }
    else {
        check.u32 = 0;
        uint32_t size = _YM_DATASIZE(packet);
        packet += 3;
        for (int i = 0; i < size; ++i) {
            check.u32 += *packet;
            ++packet;
        }
        return (check.u8[0] == *packet) ? YM_RES_OK : YM_RES_ERR_CRC;
    }
    return YM_RES_OK;
}
/* ------------------------------------------------------------------------ */
uint32_t ymodem_receive_xmodem_unsafe(bool ascii_mode )
{
    bool crc = true;
    uint32_t packet = 0;
    uint32_t res = YM_RES_TIMEOUT;
    uint8_t codes[3] = { YM_CRC, YM_ACK, YM_NAK };
    uint32_t ntries = XOPT_YMODEM_MAX_TRIES;
    uint32_t bytes_written = 0;
    uint8_t block = 0;

    _YM_DELAY(20000); // wait for about 20 seconds before starting
    while (res != YM_RES_CANCEL) {
        /*
         * sending response
         */
    	/* When block0 is active and there was a timeout, transmit
    	 * the "start" character of either "C" for crc-16 mode, or
    	 * NAK to initiate a transmission.
    	 */
        if ((res == YM_RES_TIMEOUT) && (block == 0))
            ym_putc((char)( (crc) ? codes[0] : codes[2]));
        /* send ACK when the last packet was recieved and processed ok */
        else if (res == YM_RES_OK) ym_putc(codes[1]);
        /* When the end of transfer is detected, send an acknowledge and
         * exit the loop.  The file is now complete.
         */
        else if (res == YM_RES_END_OF_TRANSFER) {
            /*
             * EOT was detected, acknowledge and then return
             */
            ym_putc((char)codes[1]);
            break;
        }
        /* When there was a timeout, just send a NAK to tell the host
         * to re-transmit.
         */
        else if (res == YM_RES_TIMEOUT) ym_putc((char)codes[2]);

        _YM_DELAY(10); // wait for 10ms (let USB catch up)
        res = ymodem_receive_xmodem_packet(crc);
        ++packet;

        if (res == YM_RES_OK) {
//            last_block = block;
            block = _ym_packet_ws[1];
        }
        else if (res == YM_RES_TIMEOUT) {
            --packet;
            if (ntries) --ntries;
            /*
             * When we are out of tries, but still waiting
             * for block 0, fallback to NAK handshake with
             * sender.
             */
            else if ((ntries == 0) && (block == 0)) {
                crc = false;
                ntries = 10;
            }
            /*
             * when we have exceeded our max timeouts
             * the sender must have quit. so, leave
             * letting the caller know that there was
             * a timeout.
             */
            else return YM_RES_TIMEOUT;
        }

        // add packet validation
        if (res == YM_RES_OK) {
            res = ymodem_verify_xmodem_packet(_ym_packet_ws, crc);
        }

        if (res == YM_RES_OK) {
            ntries = 10; // reset the timeout count-down
            /*
             * The packet has been received correctly, and
             * the packet was validated, so we are going to
             * write the data into the target file
             */
            uint32_t bw = 0;
            uint32_t btw = _YM_DATASIZE(_ym_packet_ws);
            if (ascii_mode) {
                /*
                 * in ASCII mode, we look for the CTRL-Z EOF
                 * marker and truncate the block to remove the
                 * CTRL-Z padding.
                 */
                uint32_t trim_point = btw;
                for (uint32_t idx = 0; idx < btw; ++idx) {
                    if (_ym_packet_ws[3 + idx] == YM_EOF) {
                        /*
                         * There was a ctrl-z in the data payload,
                         * but we'll keep it there unless it was
                         * padding.  the way I determine padding
                         * is to scan from the location of the
                         * ctrl-z to the end of the data size.
                         * when there are all ctrl-Z there, it was
                         * padding.  i.e. highly unlikely that
                         * a file will have a packet of ctrl-Z
                         * but it could have a lone one in the
                         * data.
                         */
                        uint32_t lookahead = idx + 1;
                        while ((lookahead < btw)
                                && (_ym_packet_ws[3 + lookahead] == YM_EOF)) {
                            ++lookahead;
                        }
                        if (lookahead >= btw) {
                            trim_point = idx;
                            idx = lookahead;
                        }
                    }
                }
                /* adjust the btw to remove the padding */
                btw = (btw > trim_point) ? trim_point : btw;
            } /* end of ascii mode handling */
            if (btw > 0) {
            	uint32_t fr = ym_stream_write(block, &_ym_packet_ws[3], btw);
            	bytes_written += bw;
            	if (fr != YM_RES_OK) {
            		/* an error has occurred, log it? */
            		_ym_fs_state = fr;
            		/* Send CANCEL to transmitter */
            		for (int i = 0; i < 8; ++i) ym_putc(YM_CAN);
            		return YM_RES_ERROR;
            	}
            }
        } /* end of packet good handling */
    } /* end of packet receive loop */

    return res;
}
/* ------------------------------------------------------------------------ */
/* YMODEM Handlers */

/* ======================================================================== */
/* XMODEM Transmitter ----------------------------------------------------- */

static uint32_t ymodem_send_xmodem_packet(uint8_t block, bool crc, uint8_t *status)
{
    uint32_t checksum;
    uint32_t bytes_read;
    uint32_t response;
    uint32_t packet_length = 1024;  //+5 for header and CRC
    uint32_t data_size = 0;

    _ym_packet_ws[1] = block;
    _ym_packet_ws[2] = 255 - block;
    uint32_t res = ym_stream_read(&_ym_packet_ws[3], packet_length, (block < 2), &bytes_read);

    if (res != YM_RES_OK) return YM_RES_ERROR;
    if (bytes_read == 0) {
        _ym_packet_ws[0] = YM_EOT;
        packet_length = 1;
    }
    else if (bytes_read <= 128) {
        /* send short end packet */
        _ym_packet_ws[0] = YM_SOH;
        packet_length = 128 + 3 + ((crc) ? 2 : 1);
        data_size = 128;
    }
    else if (bytes_read <= 1024) {
        _ym_packet_ws[0] = YM_STX;
        packet_length = 1024 + 3 + ((crc) ? 2 : 1);
        data_size = 1024;
    }

    /* add padding when required */
    for (int idx = bytes_read; idx < data_size; ++idx) {
        _ym_packet_ws[3 + idx] = 0x1A;
    }
    /* compute verification signature */
    if (crc) {
        checksum = ym_calc_crc16(&_ym_packet_ws[3], data_size, 0);
        _ym_packet_ws[3 + data_size] = (checksum >> 8) & 0xFF;
        _ym_packet_ws[4 + data_size] = (checksum & 0xFF);
    }
    else {
        checksum = 0;
        for (int idx = 0; idx < data_size; ++idx) {
            checksum += _ym_packet_ws[3 + idx];
        }
        _ym_packet_ws[3 + data_size] = checksum&0xFF;
    }
    /* send the packet (try 10 times before abort) */
    uint32_t ntries = 11;
    do {
        --ntries;
        ym_flush();
        ym_send(_ym_packet_ws, packet_length);
        response = ym_getc(XOPT_YMODEM_TIMEOUT_CHAR);
    } while ((response != YM_ACK) && (ntries > 0));

    if (status != NULL)  *status = _ym_packet_ws[0];


    if (ntries == 0) return YM_RES_TIMEOUT;
    else return YM_RES_ACK;
}
/* ------------------------------------------------------------------------ */
uint32_t ymodem_send_xmodem_unsafe( void )
{
    bool crc = true;
    uint32_t ntries = XOPT_YMODEM_MAX_TRIES;
    uint32_t response;
    uint32_t res;
    uint32_t block;
    uint8_t stat;

    /*
     * Part 1: wait for receiver's "C" or NAK
     */
    ym_flush();
    do {
        response = ym_getc(XOPT_YMODEM_TIMEOUT_CHAR*5);
        --ntries;
    } while ((response != 'C') && (response != YM_NAK) && (ntries > 0));

    if (ntries == 0) return YM_RES_TIMEOUT;
    crc = (response == 'C');

    /*
     * Part 2: send the file data
     */
    block = 1;
    do {
        res = ymodem_send_xmodem_packet(block, crc, &stat);
        ++block;
    } while ((stat != YM_EOT) && (res != YM_RES_TIMEOUT));

    return (res == YM_RES_ACK) ? YM_RES_OK : res;
}
/* ------------------------------------------------------------------------ */
uint32_t ymodem_get_error(void)
{
    return _ym_fs_state;
}
/* ------------------------------------------------------------------------ */

/* Callback API =========================================================== */
/* these functions are typically overridden by the application to defer
 * operation to the device of choosing when assigning the API for the
 * YMODEM Interface.  This protocol has standard serial APIs plus a stream
 * read/write API for loading/storing the file data from the transfer.
 */
/* ------------------------------------------------------------------------ */
#if(0)
// Write a block of data that was received
__attribute__((weak)) uint32_t ym_stream_write(uint32_t block_id, uint8_t *block, uint32_t size)
{
	(void) block_id;
	(void) block;
	(void) size;
	return YM_RES_TIMEOUT;
}
/* ------------------------------------------------------------------------ */
__attribute__((weak)) uint32_t ym_stream_read(uint8_t *block, uint32_t size, bool first, uint32_t *br)
{
	(void) block;
	(void) size;
	(void) first;
	if (br != NULL) *br = 0;  // indicate EOT

	return YM_RES_TIMEOUT;
}
/* ------------------------------------------------------------------------ */
__attribute__((weak)) void ym_flush( void )
{

}
/* ------------------------------------------------------------------------ */
__attribute__((weak)) void ym_putc(char c)
{
	(void)c;
}
/* ------------------------------------------------------------------------ */
__attribute__((weak)) uint32_t ym_getc( uint32_t timeout)
{
	(void)timeout;
	return 0xFFFFFFFF;
}
/* ------------------------------------------------------------------------ */
__attribute__((weak)) void ym_send( uint8_t* packet, uint32_t size)
{
	(void) packet;
	(void) size;
}
/* ------------------------------------------------------------------------ */
__attribute__((weak)) uint32_t ym_receive(uint8_t* packet, uint32_t size, uint32_t timeout)
{
	(void) packet;
	(void) size;
	(void) timeout;
	return 0;
}
/* ------------------------------------------------------------------------ */
#endif

/* EnD OF FILE ============================================================ */

/*
 **
 ** Copyright 2011 Intel Corporation
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **      http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 **
 ** Author:
 ** Zhang, Dongsheng <dongsheng.zhang@intel.com>
 **
 */

#include "ad_log.h"
#include "ad_protocol.h"

#define PKT_ID_SIZE		3
#define PKT_COMMAND_SIZE	1
#define PKT_CMD_OPTION_SIZE	1
#define PKT_LEN_SIZE		4
#define PKT_CRC_SIZE		2
#define PKT_HEADER_SIZE		(\
					PKT_ID_SIZE +\
					PKT_COMMAND_SIZE +\
					PKT_CMD_OPTION_SIZE +\
					PKT_LEN_SIZE\
				)

#define PACKET_ID_B0		0xAD
#define PACKET_ID_B1		0xAD
#define PACKET_ID_B2		0x00

#define CMD_WRITE_MASK		0x80
#define CMD_MAGIC		0x0A

/* es305b address as defined in Audience protocol for the MSP430 */
#define PKT_CMD_OPTION_AD_I2C_ADDRESS 0x3E

#define ARRAY_DWORD(buf,p)	((buf[p] << 24) | (buf[p + 1] << 16) | (buf[p + 2] << 8) | buf[p + 3])
#define ARRAY_WORD(buf,p)	((buf[p] << 8) | buf[p + 1])

/* Dump a buffer content in the log */
void ad_dump_buffer(unsigned char *buf, int len)
{
#define NUM_PER_ROW   8

    int i;
    int row;
    int left;

    row = len / NUM_PER_ROW;
    left = len % NUM_PER_ROW;
    /* Dump as many row as possible */
    for (i=0; i<row; i++) {
        RTRAC("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
                    *buf, *(buf+1), *(buf+2), *(buf+3), *(buf+4), *(buf+5), *(buf+6), *(buf+7));
        buf += NUM_PER_ROW;
    }
    /* Dump bytes left (min 0, max NUM_PER_ROW - 1 = 7) */
    switch(left) {
        case 1:
            RTRAC("0x%02x", *buf);
            break;
         case 2:
             RTRAC("0x%02x 0x%02x", *buf, *(buf+1));
            break;
         case 3:
             RTRAC("0x%02x 0x%02x 0x%02x", *buf, *(buf+1), *(buf+2));
            break;
         case 4:
             RTRAC("0x%02x 0x%02x 0x%02x 0x%02x", *buf, *(buf+1), *(buf+2), *(buf+3));
            break;
         case 5:
             RTRAC("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", *buf, *(buf+1), *(buf+2), *(buf+3), *(buf+4));
            break;
         case 6:
             RTRAC("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
                        *buf, *(buf+1), *(buf+2), *(buf+3), *(buf+4), *(buf+5));
            break;
         case 7:
             RTRAC("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
                        *buf, *(buf+1), *(buf+2), *(buf+3), *(buf+4), *(buf+5), *(buf+6));
            break;
         default:
            /* 0 bytes left to dump, nothing to do */
            break;
    }
}

/* Parse a packet from raw data and populate a packet_t structure */
int ad_parse_packet(struct packet_t *pkt, unsigned char *buf, int len)
{
    int ret = 0;
    int i;
    int c_crc = 0;
    int p = 0;
    int data_len;

    // Check that buf includes packet ID, command, command option and size fields
    if (len < PKT_HEADER_SIZE) {
        RERRO("ERROR: %s Invalid packet size", __func__);
        ret = S_IL;
        return ret;
    }

    // Parse the Packet ID.
    if (buf[0] == PACKET_ID_B0 && buf[1] == PACKET_ID_B1 && buf[2] == PACKET_ID_B2) {
        c_crc = PACKET_ID_B0 + PACKET_ID_B1 + PACKET_ID_B2;
        pkt->id[0] = PACKET_ID_B0;
        pkt->id[1] = PACKET_ID_B1;
        pkt->id[2] = PACKET_ID_B2;
        p += PKT_ID_SIZE;
    } else {
        RERRO("ERROR: %s packet id: [0x%02x 0x%02x 0x%02x]", __func__, buf[0], buf[1], buf[2]);
        ret = S_CNI;
        return ret;
    }

    // Parse the Command.
    if ((buf[p] & ~CMD_WRITE_MASK) == CMD_MAGIC) {
        c_crc += buf[p];
        pkt->cmd = buf[p];
        pkt->rw = buf[p] & CMD_WRITE_MASK ? FLAG_READ : FLAG_WRITE;
        p += PKT_COMMAND_SIZE;
    } else {
        RERRO("ERROR: %s cmd: [0x%02x]", __func__, buf[p]);
        ret = S_IC;
        return ret;
    }

    // Parse the Command Option.
    if (buf[p] == PKT_CMD_OPTION_AD_I2C_ADDRESS) {
        c_crc += PKT_CMD_OPTION_AD_I2C_ADDRESS;
        pkt->cmd_opt = buf[p];
        p += PKT_CMD_OPTION_SIZE;
    } else {
        RERRO("ERROR: %s addr: buf[%d]:[0x%02x]", __func__, p, buf[p]);
        ret = S_IA;
        return ret;
    }

    // Parse the len.
    data_len = ARRAY_DWORD(buf, p);
    if (pkt->rw == FLAG_WRITE) {
        // write packet, check the length.
        if ((data_len != (len - PKT_HEADER_SIZE - PKT_CRC_SIZE))) {
            RERRO("ERROR: %s data len: [0x%04x], buf[%d]:[0x%02x 0x%02x 0x%02x 0x%02x]",
                        __func__, data_len, p, buf[p], buf[p+1], buf[p+2], buf[p+3]);
            ret = S_IL;
            return ret;
        }
    }
    c_crc += (buf[p] + buf[p+1] + buf[p+2] + buf[p+3]);
    pkt->len = data_len;
    p += PKT_LEN_SIZE;

    // Parse the data, only write packet has data payload.
    pkt->data = NULL;
    if (pkt->rw == FLAG_WRITE) {
        pkt->data = &buf[p];
        for (i = 0; i < data_len; i++) {
            c_crc += buf[p + i];
        }
        p += data_len;
    }

    // Parse the crc.
    if (p + PKT_CRC_SIZE > len) {
        RERRO("ERROR: %s Invalid packet size: no CRC field", __func__);
        ret = S_IL;
        return ret;
    }
    pkt->crc_raw[0] = buf[p];
    pkt->crc_raw[1] = buf[p+1];
    pkt->crc = ARRAY_WORD(buf, p);
    if (pkt->crc != (c_crc & 0xffff)) {
        RERRO("ERROR: %s CRC: pkt->crc[0x%04x], buf[%d]:[0x%02x][0x%02x] c_crc:[0x%04x]",
                    __func__, pkt->crc, p, buf[p], buf[p+1], c_crc);
        ret = S_CRC;
        return ret;
    }

    return ret;
}

/* Populate a response_t structure */
int ad_pack_response(struct response_t *rsp, int rw, unsigned char co, int len,
    unsigned char status, unsigned char *data)
{
    int i;

    // Packet ID.
    rsp->id[0] = PACKET_ID_B0;
    rsp->id[1] = PACKET_ID_B1;
    rsp->id[2] = PACKET_ID_B2;
    rsp->crc = PACKET_ID_B0 + PACKET_ID_B1 + PACKET_ID_B2;

    // Command.
    rsp->cmd = CMD_MAGIC;
    if (rw) {
        rsp->rw = 1;
        rsp->cmd |= CMD_WRITE_MASK;
    } else {
        rsp->rw = 0;
    }
    rsp->crc += rsp->cmd;

    // Command option.
    rsp->cmd_opt = PKT_CMD_OPTION_AD_I2C_ADDRESS;
    rsp->crc += rsp->cmd_opt;

    // Length.
    rsp->len = len;
    rsp->len_raw[0] = (unsigned char)((len >> 24) & 0xFF);
    rsp->len_raw[1] = (unsigned char)((len >> 16) & 0xFF);
    rsp->len_raw[2] = (unsigned char)((len >> 8) & 0xFF);
    rsp->len_raw[3] = (unsigned char)(len & 0xFF);
    rsp->crc += (rsp->len_raw[0]  + rsp->len_raw[1] + rsp->len_raw[2] + rsp->len_raw[3]);

    // Status.
    rsp->status = status;
    rsp->crc += rsp->status;

    // Data.
    rsp->data = NULL;
    if ((len > 0) && (data != NULL)) {
        rsp->data = data;
        for (i = 0; i < rsp->len; i ++) {
            rsp->crc += rsp->data[i];
        }
    }

    // Crc.
    rsp->crc_raw[0] = (unsigned char)((rsp->crc >> 8) & 0xFF);
    rsp->crc_raw[1] = (unsigned char)(rsp->crc & 0xFF);

    return 0;
}

/* Serialize a response_t structure to a byte array to be sent as raw data */
int ad_response_serialize(struct response_t *rsp, unsigned char *buf, int len)
{
    int p = 0;
    int i = 0;

    if (len < rsp->len + PKT_HEADER_SIZE + PKT_CRC_SIZE) {
        RERRO("ERROR: %s Invalid buf size", __func__);
        return p;
    }

    // Packet ID.
    buf[p++] = rsp->id[0];
    buf[p++] = rsp->id[1];
    buf[p++] = rsp->id[2];

    // Command.
    buf[p++] = rsp->cmd;

    // Command option.
    buf[p++] = rsp->cmd_opt;

    // Len.
    buf[p++] = rsp->len_raw[0];
    buf[p++] = rsp->len_raw[1];
    buf[p++] = rsp->len_raw[2];
    buf[p++] = rsp->len_raw[3];

    // Status.
    buf[p++] = rsp->status;

    // Data. only read response has the data.
    if ((rsp->rw == 1) && (rsp->len > 0) && (rsp->data != NULL)) {
        //memcpy(rsp->data, &buf[p], rsp->len);
        for (i=0; i < rsp->len; i++)
            buf[p+i] = rsp->data[i];
        p += rsp->len;
    }

    // Crc.
    buf[p++] = rsp->crc_raw[0];
    buf[p++] = rsp->crc_raw[1];

    return p;
}


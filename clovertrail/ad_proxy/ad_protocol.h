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
#ifndef _AD_PROTOCOL_H_
#define _AD_PROTOCOL_H_

#define AD_I2C_ADDRESS 0x3E

enum rw_flag {
    FLAG_WRITE = 0,
    FLAG_READ
};

struct packet_t {
    unsigned char id[3];
    unsigned char cmd;
    enum rw_flag rw;
    unsigned char cmd_opt;
    int len;
    unsigned char *data;
    unsigned char crc_raw[2];
    int crc;
};

struct response_t {
    unsigned char id[3];
    unsigned char cmd;
    unsigned char cmd_opt;
    unsigned char len_raw[4];
    unsigned char status;
    unsigned char crc_raw[2];

    unsigned char *data;
    int rw;
    int len;
    int crc;
};

enum {
    S_OK = 0,
    S_CNI, // Command Not Implemented
    S_BE, // Bus error
    S_IC, // Invalid Command
    S_IP, // Invalid Parameter
    S_IL, // Invalid Length
    S_TO, // Time Out
    S_IA, // Invalid Address
    S_CRC, //CRC error
};

void ad_dump_buffer(unsigned char *buf, int len);
int ad_parse_packet(struct packet_t *pkt, unsigned char *buf, int len);
int ad_pack_response(struct response_t *rsp, int rw, unsigned char co, int len,
    unsigned char status, unsigned char *data);
int ad_response_serialize(struct response_t *rsp, unsigned char *buf, int len);

#endif //_AD_PROTOCOL_H_

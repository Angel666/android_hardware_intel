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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <termios.h>
#include <pthread.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/ioctl.h>

#include "ad_log.h"
#include "ad_i2c.h"
#include "ad_usb_tty.h"
#include "ad_protocol.h"

#define AD_VERSION "V1.1"


#define TTY_RDY_WAIT	100 /* in ms */

enum {
    ID_STD = 0,
    ID_TTY,
    ID_NB
};

enum {
    DUMP_NONE,
    DUMP_PACKET,
    DUMP_PAYLOAD,
};

#define ACM_TTY "/dev/ttyGS0"

#define CMD_SIZE 64
char cmd[CMD_SIZE];
volatile int quit = 0;

int fds[ID_NB] = {-1};
struct pollfd polls[ID_NB];

pthread_t proxy_thread;

#define MAX_PACKET_SIZE  1024
unsigned char packet_raw[MAX_PACKET_SIZE + 1];
unsigned char response_raw[MAX_PACKET_SIZE + 1];

unsigned int trans_id = 0;
volatile int dump_level = DUMP_PACKET;

static void ad_signal_handler(int signal)
{
    RDBUG("Signal [%d]", signal);
    /* Signal exit to the proxy thread */
    quit = 1;
    /* Wait for the proxy thread exit */
    pthread_join(proxy_thread, NULL);

    if (polls[ID_TTY].fd >= 0) {
        close(polls[ID_TTY].fd);
    }

    ad_i2c_exit();

    exit(0);
}

static void * ad_proxy_worker(void* data)
{
    int ret;
    int pollResult;
    int readSize;
    struct packet_t pkt;
    struct response_t rsp;
    int writeSize;
    unsigned char status;
    unsigned char buff[MAX_PACKET_SIZE];

    RTRAC("Start proxy worker thread\n");
    while (!quit) {
        // poll fds.
        RDBUG("Poll");
        polls[ID_TTY].events = POLLIN;
        polls[ID_TTY].revents = 0;
        pollResult = poll(&polls[ID_TTY], 1, 1000);
        if (pollResult <= 0) {
            RERRO("pollResult %d errno %d", pollResult, errno);
            continue;
        }

        // read events from available fds.
        if (polls[ID_TTY].revents & POLLIN) {
            RTRAC("Try to read data...");
            readSize = read(polls[ID_TTY].fd, packet_raw, MAX_PACKET_SIZE);
            if (readSize <= 0) {
                usleep(100000);
                RDBUG("Close & re open TTY");
                close(polls[ID_TTY].fd);
                polls[ID_TTY].fd = -1;
                // try to re-open the tty.
                polls[ID_TTY].fd = ad_open_tty(ACM_TTY,  B115200);
                if (polls[ID_TTY].fd < 0) {
                    RERRO("ERROR: %s open %s failed (%s)\n",  __func__, ACM_TTY, strerror(errno));
                    /* Cannot continue without the TTY */
                    break;
                }
            } else {
                packet_raw[readSize] = 0;

                if (dump_level == DUMP_PACKET) {
                    RDBUG("[%d]: RECV [%d] bytes:", trans_id, readSize);
                    ad_dump_buffer(packet_raw, readSize);
                }

                // parse the packet.
                if (ad_parse_packet(&pkt, packet_raw, readSize)) {
                    continue;
                }

                if (dump_level == DUMP_PAYLOAD) {
                    if (!pkt.rw && pkt.len) {
                        RDBUG("[%d]: [%s] [%d]", trans_id, pkt.rw?"r":"w", pkt.len);
                        ad_dump_buffer(pkt.data, pkt.len);
                    }
                }

                // route the packet to audience.
                if (pkt.rw) {
                    // read from audience.
                    ret = ad_i2c_read(buff, pkt.len);
                    if (ret != pkt.len) {
                        status = S_BE;
                        pkt.len = 0;
                        pkt.data = NULL;
                    } else {
                        status =  S_OK;
                        pkt.data = buff; // for read, point to the read data.
                    }
                } else {
                    // write.
                    status = S_OK;
                    if (ad_i2c_write(pkt.data, pkt.len) != pkt.len)
                        status = S_BE;
                    pkt.data = NULL; // for write, no data in the response.
                }

                // pack the response.
                if (ad_pack_response(&rsp, pkt.rw, AD_I2C_ADDRESS, pkt.len, status, pkt.data)) {
                    continue;
                }

                // write back the response.
                writeSize = ad_response_serialize(&rsp, response_raw, MAX_PACKET_SIZE);
                if (writeSize > 0) {
                    if ((ret = write(polls[ID_TTY].fd, response_raw, writeSize))  != writeSize) {
                        RERRO("Error: %s write response [%s]", __func__, strerror(errno));
                        continue;
                    }

                    // dump the packets.
                    if (dump_level == DUMP_PACKET) {
                        RDBUG("[%d]: SEND [%d] bytes", trans_id, writeSize);
                        ad_dump_buffer(response_raw, writeSize);
                    } else if (dump_level == DUMP_PAYLOAD) {
                        if (rsp.rw && rsp.len > 0 && rsp.data != NULL) {
                            RDBUG("[%d]: [%s] [%d]", trans_id, rsp.rw?"r":"w", rsp.len);
                            ad_dump_buffer(rsp.data, rsp.len);
                        }
                    }
                }

                // success got one packet.
                trans_id++;
            }
        }
    }

    RDBUG("Exit Working Thread");
    pthread_exit(NULL);
    /* To avoid warning, must return a void * */
    return NULL;
}

static void dump_command_help(void)
{
    fprintf(stderr, "Commands:\n" \
                    "Q\t\tQuit\n" \
                    "Dn\t\tSet dump level to n (n in [0..2] 0:No dump; 1:dump packet; 2:dump payload)\n" \
                    "V\t\tPrint ad_proxy version\n" \
                    "W0xDEADBEEF\tWrite 0xDEADBEEF to es305b\n" \
                    "R\t\tRead a 32bits word from es305b\n");
}

int main(int argc, char *argv[])
{
#define ARG_SIZE	16
    int i = 0;
    int tmp;
    char p[ARG_SIZE];
    int ret = 0;
    int readSize;
    int pollResult;
    int i2c_delay = -1;
    struct sigaction sigact;
    pthread_attr_t attr;
    int thread_started = 0;

    RTRAC("-->ad_proxy startup");

    // Set signal handler
    if (sigemptyset(&sigact.sa_mask) == -1) {
        exit(-1);
    }
    sigact.sa_flags = 0;
    sigact.sa_handler = ad_signal_handler;

    if (sigaction(SIGHUP, &sigact, NULL) == -1) {
        exit(-1);
    }
    if (sigaction(SIGTERM, &sigact, NULL) == -1) {
        exit(-1);
    }

    argc--;
    i++;
    do {
        // Parse the i2c operation wait parameter.
        if (argc > 0) {
            strncpy(p, argv[i], ARG_SIZE - 1);
            p[ARG_SIZE - 1] = '\0';
            if (p[0] ==  '-' && p[1] == 'i' && p[2] == 'w') {
                tmp = atoi(p+3);
                if (tmp >= 0 && tmp <= AD_I2C_OP_MAX_DELAY)
                    i2c_delay = tmp;
                RTRAC("i2c operation wait request = %s]; i2c operation wait set = %d", p+3, i2c_delay);
            // Parse the dump level.
            } else if (p[0] ==  '-' && p[1] == 'd') {
                p[3] = 0;
                tmp = atoi(p+2);
                if (tmp >= DUMP_NONE && tmp <= DUMP_PAYLOAD)
                    dump_level = tmp;
                RTRAC("Dump level request = %s; Dump level set = %d", p+2, dump_level);
            // Parse the usage request.
            } else {
                fprintf(stdout, "Audience proxy %s\nUsage:\n", AD_VERSION);
                fprintf(stdout, "-iw[i2c operation wait (0 to %d us)]\n", AD_I2C_OP_MAX_DELAY);
                fprintf(stdout, "-d[dump level 0:no dump; 1:dump packet; 2:dump payload]\n");
                goto EXIT;
            }
            argc--;
            i++;
        }
    } while (argc > 0);

    // set the standard input poll id.
    polls[ID_STD].fd = 0;
    polls[ID_STD].events = POLLIN;
    polls[ID_STD].revents = 0;

    // open the acm tty.
    polls[ID_TTY].fd = ad_open_tty(ACM_TTY,  B115200);
    if (polls[ID_TTY].fd < 0) {
        RERRO("ERROR: %s open %s failed (%s)\n",  __func__, ACM_TTY, strerror(errno));
        ret = -1;
        goto EXIT;
    }

    // open the audience device node.
    if (ad_i2c_init(i2c_delay) < 0) {
        RERRO("ERROR: %s open %s failed (%s)\n", __func__, AD_DEV_NODE, strerror(errno));
        goto EXIT;
    }

    // start the proxy thread.
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    if (pthread_create(&proxy_thread, &attr, ad_proxy_worker, NULL) != 0) {
        RERRO("ERROR: %s Thread creation has failed (%s)\n", __func__, strerror(errno));
        goto EXIT;
    }
    pthread_attr_destroy(&attr);
    thread_started = 1;

    // main thread to wait for user's command.
    cmd[0] = 0;
    while(!quit) {

        fprintf(stderr, ">:");
        pollResult = poll(&polls[ID_STD], 1, -1);
        if (pollResult <= 0) {
            if (errno != EINTR) {
                RERRO("ERROR: %s poll failed (%s)\n", __func__, strerror(errno));
            }
        }

        if (polls[ID_STD].revents & POLLIN) {
            readSize = read(polls[ID_STD].fd, cmd, CMD_SIZE);
            if (readSize > 0) {
                cmd[readSize-1] = 0;
            } else {
                continue;
            }
        }

        switch (cmd[0]) {
        case 'q':
        case 'Q':
            fprintf(stderr, "Quit\n");
            quit = 1;
            break;
        case 'd':
        case 'D':
            cmd[2] = 0;
            tmp = atoi(&cmd[1]);
            if (tmp >= DUMP_NONE && tmp <= DUMP_PAYLOAD) {
                fprintf(stderr, "Dump level: %d\n", tmp);
                dump_level = tmp;
            } else {
                fprintf(stderr, "Invalid dump level\n\n");
                dump_command_help();
            }
            break;
         case 'v':
         case 'V':
            fprintf(stderr, "%s\n", AD_VERSION);
            break;
         case 'w':
         case 'W':
            {
                unsigned char status = 0;
                long long int w_d = 0;
                unsigned char w_buf[4];

                cmd[11] = 0;
                if (strlen(cmd + 1) == 10) {
                    w_d = strtoll(cmd+1, NULL, 16);
                    if (w_d != LLONG_MIN && w_d != LLONG_MAX && w_d >= 0) {
                        w_buf[3] = (w_d & 0xff);
                        w_buf[2] = (w_d >> 8) & 0xff;
                        w_buf[1] = (w_d >> 16) & 0xff;
                        w_buf[0] = (w_d >> 24) & 0xff;

                        fprintf(stderr, "W: 0x%02x%02x%02x%02x\n", w_buf[0], w_buf[1], w_buf[2], w_buf[3]);
                        status = ad_i2c_write(w_buf, 4);
                        fprintf(stderr, "W: status %s (%d)\n", status == 4 ? "ok":"error", status);
                        break;
                    }
                }
                fprintf(stderr, "Invalid write command.\n\n");
                dump_command_help();
            }
            break;
         case 'r':
         case 'R':
            {
                unsigned char r_buf[4];
                unsigned char status;
                status = ad_i2c_read(r_buf, 4);
                if (status == 4)
                    fprintf(stderr, "R: 0x%02x%02x%02x%02x\n", r_buf[0], r_buf[1], r_buf[2], r_buf[3]);
                else
                    fprintf(stderr, "Read error (%d)\n", status);
            }
            break;
         default:
            dump_command_help();
            break;
        }
    }

EXIT:
    /* Wait for the proxy thread exit */
    if (thread_started) {
        RDBUG("Waiting working thread");
        pthread_join(proxy_thread, NULL);
    }

    if (polls[ID_TTY].fd >= 0) {
        close(polls[ID_TTY].fd);
    }
    ad_i2c_exit();

    RTRAC("-->ad_proxy exit(%d)", ret);

    pthread_exit(NULL);
    return ret;
}

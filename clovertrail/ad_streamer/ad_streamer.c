/*
 * **
 * ** Copyright 2011 Intel Corporation
 * **
 * ** Licensed under the Apache License, Version 2.0 (the "License");
 * ** you may not use this file except in compliance with the License.
 * ** You may obtain a copy of the License at
 * **
 * **      http://www.apache.org/licenses/LICENSE-2.0
 * **
 * ** Unless required by applicable law or agreed to in writing, software
 * ** distributed under the License is distributed on an "AS IS" BASIS,
 * ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * ** See the License for the specific language governing permissions and
 * ** limitations under the License.
 * **
 * ** Author:
 * ** Lee, Jhin <jhinx.lee@intel.com>
 * **
 * */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <semaphore.h>

#include "hardware_legacy/power.h"

#define ES305_DEVICE_PATH	"/dev/audience_es305"

#define ES305_CH_PRI_MIC	0x1
#define ES305_CH_SEC_MIC	0x2
#define ES305_CH_CLEAN_SPEECH	0x4
#define ES305_CH_FAR_END_IN	0x8
#define ES305_CH_FAR_END_OUT	0x10

#define ES305_CHANNEL_SELECT_FIELD 3

#define DEFAULT_OUTPUT_FILE	"/data/a1026_stream.bin"

//Total byte of buffer size would be ES305_BUFFER_SIZE * ES305_BUFFER_COUNT
//Audience send 328 bytes for each frame. 164 bytes are half of the frame size.
//It also need to be less than 256 bytes as the limitation of I2C driver.
#define ES305_BUFFER_SIZE		164
/* ES305_BUFFER_COUNT must be a power of 2 */
#define ES305_BUFFER_COUNT		256

#define false	0
#define true	1

#define MAX_FILE_PATH_SIZE 80
#define READ_ACK_BUF_SIZE 8

//Time out used by the run_writefile thread to detect
//that the capture thread has stopped and does not produce
//buffer anymore. Assuming a bandwith of about 70KB/s for the
//catpure thread, and a buffer size of 1024B, the capture thread
//should produce a buffer about every 14 ms. Since the time out
//value must be in seconds, the closest value is 1 second.
#define READ_TIMEOUT_IN_SEC 1
//wait for streaming capture starts
#define START_TIMEOUT_IN_SEC 1
//Waiting until all audience stream is flushed in micro seconds
#define THREAD_EXIT_WAIT_IN_US 100000

static const unsigned char startStreamCmd[4] = { 0x80, 0x25, 0x00, 0x01 };
static const unsigned char stopStreamCmd[4] = { 0x80, 0x25, 0x00, 0x00 };
static const unsigned char setOutputKnownSignal[4] = { 0x80, 0x1E, 0x00, 0x05 };
static unsigned char setChannelCmd[4] = { 0x80, 0x28, 0x00, 0x03 };

// Globals.....
static int fd = -1;
static FILE *outfile = NULL;
static volatile int stop_requested = false;
static unsigned int written_bytes = 0;
static unsigned int read_bytes = 0;
static unsigned char data[ES305_BUFFER_COUNT][ES305_BUFFER_SIZE];

static sem_t sem_read;
static sem_t sem_start;

static char lockid[32];

static volatile long int cap_idx = 0;
static volatile long int wrt_idx = 0;

// Thread to keep track of time
void* run_capture(void *ptr) {
	int index = 0;
	struct timespec ts;
	int buffer_level;

	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += START_TIMEOUT_IN_SEC;
	if(sem_timedwait(&sem_start, &ts)){
		printf("Error: read start timed out.\n");
		stop_requested = true;
		return NULL;
	}
	while (!stop_requested) {
		read(fd, data[index++], ES305_BUFFER_SIZE);
		cap_idx++;
		if(UINT_MAX - read_bytes >= ES305_BUFFER_SIZE)
			read_bytes += ES305_BUFFER_SIZE;
		else
			printf("read bytes are overflowed.\n");
		index &= ES305_BUFFER_COUNT-1;
		buffer_level = cap_idx + wrt_idx;
		if(buffer_level > ES305_BUFFER_COUNT){
			printf("Warning: buffer overflow (%d/%d)\n", buffer_level, ES305_BUFFER_COUNT);
		}
		sem_post(&sem_read);
	}
	return NULL;
}

void* run_writefile(void *ptr) {
	int index = 0;
	struct timespec ts;

	while (true) {
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += READ_TIMEOUT_IN_SEC;
		if(sem_timedwait(&sem_read, &ts)){
			if(read_bytes > written_bytes){
				printf("Error:timed out.\n");
				stop_requested = true;
			}
			return NULL;
		}
		fwrite(data[index++], sizeof(unsigned char),ES305_BUFFER_SIZE, outfile);
		wrt_idx--;
		if(UINT_MAX - written_bytes >= ES305_BUFFER_SIZE)
			written_bytes+=ES305_BUFFER_SIZE;
		else
			printf("written bytes are overflowed\n");
		index &= ES305_BUFFER_COUNT-1;
	}
	return NULL;
}

void cleanup() {
	release_wake_lock(lockid);
	if (fd > 0)
		close(fd);
	if (outfile != NULL)
		fclose(outfile);
}

int check_numeric(char *number){
	int index = 0;

	while(number[index] != '\0'){
		if(!isdigit(number[index++]))
			return -1;
	}
	return 0;
}

int display_help(){
	printf("Format: [flag] [seconds] [optional:output file path/filename]\n");
	printf("flag = channel options:\n");
	printf("	Primary Mic   = %d\n", ES305_CH_PRI_MIC);
	printf("	Secondary Mic = %d\n", ES305_CH_SEC_MIC);
	printf("	Clean Speech  = %d\n", ES305_CH_CLEAN_SPEECH);
	printf("	Far End In    = %d\n", ES305_CH_FAR_END_IN);
	printf("	Far End out   = %d\n", ES305_CH_FAR_END_OUT);
	printf("Usage of flag: Capture Primary Mic and Far End In, set FLAG to 9 (1 + 8)\n");
	printf("Default output file path is /data/a1026_stream.bin\n");
	printf("Example: ad_streamer 9 10 /sdcard/aud_stream.bin\n");
	return 0;
}
int main(int argc, char **argv) {
	int rc;

	unsigned char buf[READ_ACK_BUF_SIZE];
	char fname[MAX_FILE_PATH_SIZE];

	struct sched_param param;
	pthread_attr_t thread_attr;

	pthread_t pt_capture;
	pthread_t pt_fwrite;

	int duration_in_sec = 0;
	int channel_flag = 0;

	if(argc < 3){
		display_help();
		return -1;
	}

	if(check_numeric(argv[1])){
		display_help();
		printf("\n*Please use valid numeric value for channel flag\n");
		return -1;
	}

	if(check_numeric(argv[2])){
		display_help();
		printf("\n*Please use valid numeric value for seconds\n");
		return -1;
	}

	// Set streaming channel flag
	channel_flag = atoi(argv[1]);

	// Cap the duration_in_sec to capture samples (in seconds)
	duration_in_sec = atoi(argv[2]);

	// Initialize
	fd = open(ES305_DEVICE_PATH, O_RDWR | O_NONBLOCK, 0);

	if (fd < 0) {
		printf("Cannot open %s\n", ES305_DEVICE_PATH);
		return -1;
	}

	// Set output file
	if (argc > 3) {
		snprintf(fname, MAX_FILE_PATH_SIZE, "%s", argv[3]);
	} else {
		strncpy(fname, DEFAULT_OUTPUT_FILE, MAX_FILE_PATH_SIZE - 1);
		fname[MAX_FILE_PATH_SIZE-1]='\0';
	}

	outfile = fopen(fname, "w+b");
	if (!outfile) {
		printf("Cannot open output file %s\n", fname);
		cleanup();
		return -1;
	}
	printf("Outputing raw streaming data to %s\n", fname);

	// Set streaming channels
	setChannelCmd[ES305_CHANNEL_SELECT_FIELD] = channel_flag;
	printf("Setting streaming channels: 0x%02x\n", setChannelCmd[ES305_CHANNEL_SELECT_FIELD]);

	rc = write(fd, setChannelCmd, sizeof(setChannelCmd));
	if (rc < 0) {
		printf("audience set channel command failed: %d\n", rc);
		cleanup();
		return rc;
	}

	// Initialze semaphore and threads
	if(sem_init(&sem_read,0,0) != 0){
		printf("Initialzing semaphore failed\n");
		cleanup();
		return -1;
	}
	if(sem_init(&sem_start,0,0) != 0){
		printf("Initialzing semaphore failed\n");
		cleanup();
		return -1;
	}
	if(pthread_attr_init(&thread_attr) != 0){
		cleanup();
		return -1;
	}
	if(pthread_attr_setschedpolicy(&thread_attr, SCHED_RR) != 0){
		cleanup();
		return -1;
	}
	param.sched_priority = sched_get_priority_max(SCHED_RR);
	if(pthread_attr_setschedparam (&thread_attr, &param) != 0){
		cleanup();
		return -1;
	}
	if(pthread_create(&pt_capture, &thread_attr, run_capture, NULL) != 0){
		printf("Initialzing read thread failed\n");
		cleanup();
		return -1;
	}
	if(pthread_create(&pt_fwrite, NULL, run_writefile, NULL) != 0){
		printf("Initialzing write thread failed\n");
		cleanup();
		return -1;
	}
	acquire_wake_lock(PARTIAL_WAKE_LOCK, lockid);

	// Start Streaming
	rc = write(fd, startStreamCmd, sizeof(startStreamCmd));
	if (rc < 0) {
		printf("audience start stream command failed: %d\n", rc);
		cleanup();
		return rc;
	}

	// Read back the cmd ack
	read(fd, buf, READ_ACK_BUF_SIZE);

	// Let thread start recording
	sem_post(&sem_start);

	printf("\nAudio streaming started, capturing for %d seconds\n", duration_in_sec);
	while (duration_in_sec > 0 && stop_requested != true) {
		sleep(1);
		duration_in_sec -= 1;
	}

	// Stop Streaming
	rc = write(fd, stopStreamCmd, sizeof(stopStreamCmd));
	if (rc < 0) {
		printf("audience stop stream command failed: %d\n", rc);
		cleanup();
		return rc;
	}

	// Because there is no way to know whether audience I2C buffer is empty, sleep is required.
	usleep(THREAD_EXIT_WAIT_IN_US);
	stop_requested = true;

	printf("Stopping capture.\n");
	pthread_join(pt_capture, NULL);
	printf("Capture stopped.\n");
	pthread_join(pt_fwrite, NULL);

	printf("Total of %d bytes read\n", written_bytes);

	cleanup();
	return 0;
}

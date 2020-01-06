#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h> 

char STRINGS[4][65535] = { "loldongs", "fbot", "satori", "fuck" };

int main() {

	u_int32_t MAX_PID;
	FILE *PID_MAX_FILE;
	PID_MAX_FILE = fopen("/proc/sys/kernel/pid_max", "r");
  	fscanf(PID_MAX_FILE, "%u", &MAX_PID);
  	fclose(PID_MAX_FILE);
	u_int32_t RUNNING_PIDS[65535] = { 0 };
	u_int16_t RUNNING_PROCESSES = 0;
	u_int32_t CURRENT_PID = getpid();

	for(u_int32_t SCAN_PID = 1; SCAN_PID < MAX_PID; SCAN_PID++) {
		if(SCAN_PID != CURRENT_PID && kill(SCAN_PID, 0) == 0) {
			if(RUNNING_PIDS[RUNNING_PROCESSES] == 0) {
				RUNNING_PIDS[RUNNING_PROCESSES] = SCAN_PID;
				RUNNING_PROCESSES++;
			}
		}
	}

	for(u_int32_t SCAN_PID = 0; SCAN_PID < RUNNING_PROCESSES; SCAN_PID++) {
		char PROC_PATH[65535];
		char EXE_PATH[65535];
		snprintf(PROC_PATH, 65535, "/proc/%u/exe", RUNNING_PIDS[SCAN_PID]);
		readlink(PROC_PATH, EXE_PATH, 65535);
		if(strstr(EXE_PATH, "(deleted)") != 0) {
			printf("Killing process with deleted executable\n");
			printf("%s\n", EXE_PATH);
			kill(RUNNING_PIDS[SCAN_PID], 9);
		}
		FILE *EXE_FILE = fopen(EXE_PATH, "r");
		if(EXE_FILE != NULL) {
			fseek(EXE_FILE, 0, SEEK_END);
			u_int32_t EXE_SIZE = ftell(EXE_FILE);
			fseek(EXE_FILE, 0, SEEK_SET);
			char *BINARY_BUFFER = malloc(EXE_SIZE);
			fread(BINARY_BUFFER, EXE_SIZE, 1, EXE_FILE);
			fclose(EXE_FILE);
			for(u_int32_t READ_CHARACTERS = 0; READ_CHARACTERS < EXE_SIZE; READ_CHARACTERS++) {
				for(u_int16_t SCANNED_STRING = 0; SCANNED_STRING < sizeof(STRINGS) / sizeof(STRINGS[0]); SCANNED_STRING++) {
					u_int16_t STRING_LENGTH = strlen(STRINGS[SCANNED_STRING]);
					if(STRINGS[SCANNED_STRING][0] == BINARY_BUFFER[READ_CHARACTERS]) {
						u_int16_t MATCHED_CHARS = 1;
						for(u_int16_t SCANNED_CHAR = 1; SCANNED_CHAR < STRING_LENGTH; SCANNED_CHAR++) {
							if(STRINGS[SCANNED_STRING][SCANNED_CHAR] == BINARY_BUFFER[READ_CHARACTERS + SCANNED_CHAR]) {
								MATCHED_CHARS++;
							} else {
								break;
							}
							if(MATCHED_CHARS == STRING_LENGTH && kill(RUNNING_PIDS[SCAN_PID], 0) == 0) {
								printf("Killing process containing matching series of bytes and deleting executable\n");
								printf("%s\n", EXE_PATH);
								kill(RUNNING_PIDS[SCAN_PID], 9);
								remove(EXE_PATH);
							}
						}
					}
				}
			}
			free(BINARY_BUFFER);
		}
		memset(EXE_PATH, 0, 65535);
	}
 	return 0;
}

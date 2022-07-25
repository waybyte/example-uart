/**
 * @file main.c
 * @brief Exmaple application to demonstrate UART usage via standard system calls
 * @version 0.1
 * @date 2022-07-05
 * 
 * @copyright Copyright (c) 2022 Waybyte Solutions
 * 
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/select.h>

#include <lib.h>
#include <ril.h>
#include <os_api.h>

#ifdef SOC_RDA8910
#define STDIO_PORT "/dev/ttyUSB0"
#define PORT1 "/dev/ttyS0"
#define PORT2 "/dev/ttyS1"
#else
#define STDIO_PORT "/dev/ttyS0"
#define PORT1 "/dev/ttyS1"
#define PORT2 "/dev/ttyS2"
#endif

/**
 * URC Handler
 * @param param1	URC Code
 * @param param2	URC Parameter
 */
static void urc_callback(unsigned int param1, unsigned int param2)
{
	switch (param1) {
	case URC_SYS_INIT_STATE_IND:
		if (param2 == SYS_STATE_SMSOK) {
			/* Ready for SMS */
		}
		break;
	case URC_SIM_CARD_STATE_IND:
		switch (param2) {
		case SIM_STAT_NOT_INSERTED:
			debug(DBG_OFF, "SYSTEM: SIM card not inserted!\n");
			break;
		case SIM_STAT_READY:
			debug(DBG_INFO, "SYSTEM: SIM card Ready!\n");
			break;
		case SIM_STAT_PIN_REQ:
			debug(DBG_OFF, "SYSTEM: SIM PIN required!\n");
			break;
		case SIM_STAT_PUK_REQ:
			debug(DBG_OFF, "SYSTEM: SIM PUK required!\n");
			break;
		case SIM_STAT_NOT_READY:
			debug(DBG_OFF, "SYSTEM: SIM card not recognized!\n");
			break;
		default:
			debug(DBG_OFF, "SYSTEM: SIM ERROR: %d\n", param2);
		}
		break;
	case URC_GSM_NW_STATE_IND:
		debug(DBG_OFF, "SYSTEM: GSM NW State: %d\n", param2);
		break;
	case URC_GPRS_NW_STATE_IND:
		break;
	case URC_CFUN_STATE_IND:
		break;
	case URC_COMING_CALL_IND:
		debug(DBG_OFF, "Incoming voice call from: %s\n", ((struct ril_callinfo_t *)param2)->number);
		/* Take action here, Answer/Hang-up */
		break;
	case URC_CALL_STATE_IND:
		switch (param2) {
		case CALL_STATE_BUSY:
			debug(DBG_OFF, "The number you dialed is busy now\n");
			break;
		case CALL_STATE_NO_ANSWER:
			debug(DBG_OFF, "The number you dialed has no answer\n");
			break;
		case CALL_STATE_NO_CARRIER:
			debug(DBG_OFF, "The number you dialed cannot reach\n");
			break;
		case CALL_STATE_NO_DIALTONE:
			debug(DBG_OFF, "No Dial tone\n");
			break;
		default:
			break;
		}
		break;
	case URC_NEW_SMS_IND:
		debug(DBG_OFF, "SMS: New SMS (%d)\n", param2);
		/* Handle New SMS */
		break;
	case URC_MODULE_VOLTAGE_IND:
		debug(DBG_INFO, "VBatt Voltage: %d\n", param2);
		break;
	case URC_ALARM_RING_IND:
		break;
	case URC_FILE_DOWNLOAD_STATUS:
		break;
	case URC_FOTA_STARTED:
		break;
	case URC_FOTA_FINISHED:
		break;
	case URC_FOTA_FAILED:
		break;
	case URC_STKPCI_RSP_IND:
		break;
	default:
		break;
	}
}

/**
 * Uart 1 Echo Task
 * 
 * @param arg	Task Argument
 */
static void uart1_echo_task(void *arg)
{
	int fd;
	struct termios t;

	fd = open(PORT1, O_RDWR);
	if (fd < 0) {
		printf("UART1: Error opening %s, err %d\n", PORT1, errno);
		return;
	}

	tcgetattr(fd, &t);
	cfsetspeed(&t, B9600);
	tcsetattr(fd, TCSANOW, &t);
	printf("UART1: %s open @ 9600\n", PORT1);

	/* echo loop */
	while (1) {
		int ret;
		char ch;

		/* read data from uart */
		ret = read(fd, &ch, 1);
		if (ret < 0) {
			printf("UART1: fail to read from uart: %d\n", errno);
			break;
		}

		/* echo back to uart */
		write(fd, &ch, 1);
	}

	close(fd);
}

/**
 * Uart 2 echo task with timeout example using select system call
 * 
 * @param arg	Task Argument
 */
static void uart2_echo_task(void *arg)
{
	int fd;

	fd = open(PORT2, O_RDWR);
	if (fd < 0) {
		printf("UART2: Error opening %s, err %d\n", PORT2, errno);
		return;
	}

	printf("UART2: %s open @ 115200\n", PORT2);

	while (1) {
		int ret;
		fd_set set;
		struct timeval timeout;
		char rdbuf[128];

		/* 5 sec timeout */
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		FD_ZERO(&set); /* clear the set */
		FD_SET(fd, &set); /* add our file descriptor to the set */
		ret = select(fd + 1, &set, NULL, NULL, &timeout);

		if (ret > 0) {
			ret = read(fd, rdbuf, sizeof(rdbuf));
			if (ret < 0) {
				printf("UART2: fail to read from uart: %d\n", errno);
				break;
			}
			/* echo whatever read */
			write(fd, rdbuf, ret);
		} else if (ret == -1) {
			printf("UART2: Select error: %d\n", errno);
			break;
		} else {
			/* timeout */
			write(fd, "\r\nUART2: No data for 5 sec\r\n",
					strlen("\r\nUART2: No data for 5 sec\r\n"));
			printf("\r\nUART2: No data for 5 sec\r\n");
		}
	}
}

/**
 * Application main entry point
 */
int main(int argc, char *argv[])
{
	/*
	 * Initialize library and Setup STDIO
	 */
	logicrom_init(STDIO_PORT, urc_callback);

	printf("System Ready\n");

	/* Create Application tasks */
	os_task_create(uart1_echo_task, NULL, FALSE);
	os_task_create(uart2_echo_task, NULL, FALSE);

	printf("System Initialization finished\n");

	while (1) {
		/* Main task */
		sleep(1);
	}
}

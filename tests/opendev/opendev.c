#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>

#define MAX_PATH 256

#define	OPEN_HANDLE 0x3d
#define	CLOSE_HANDLE 0x3e

#define FAILURE 0
#define SUCCESS (!FAILURE)


/* opens device, 
   prepends device name with specified drive letter, 
   returns handle to open device or -1 on error 
 */
int open_dev(const char drive_letter, const char *device_name)
{
	union REGS regs;
	struct SREGS sregs;
	char buffer[MAX_PATH];

	sprintf(buffer, "%c:%s", drive_letter, device_name);
	
	printf("Opening %s\n", buffer);

	regs.h.ah = OPEN_HANDLE;
	regs.h.al = 0;
	regs.x.dx = FP_OFF((void far *)buffer);
	sregs.ds = FP_SEG((void far *)buffer);
	intdosx(&regs, &regs, &sregs);
	if (regs.x.cflag)
		return -1;
	return regs.x.ax;
}

/* close device using file descriptor returned from open_dev
 */
void close_dev(int fd)
{
	union REGS regs;

	regs.h.ah = CLOSE_HANDLE;
	regs.x.bx = fd;
	intdos(&regs, &regs);
}

int test_device(const char drive_letter, const char *device_name, int should_fail)
{
	int fd;
	fd = open_dev(drive_letter, device_name);
	if (fd != -1) 
		close_dev(fd);

	if ( ((fd == -1) && should_fail) || ((fd != -1) && !should_fail) )
	{
		printf("SUCCESS - ");
		if (should_fail)
			printf("open failed\n");
		else
			printf("open succeeded\n");
		return SUCCESS;
	}
	else 
	{
		printf("FAILURE - ");
		if (should_fail)
			printf("open succeeded\n");
		else
			printf("open failed\n");
		return FAILURE;
	}
}

int do_tests_on(const char *device_name)
{
	int status = SUCCESS;
	char buffer[MAX_PATH];
	const char current_drive = (char)('A'+_getdrive()-1);

	sprintf(buffer, "%s", device_name);
	if (!test_device('@', buffer,0)) status = FAILURE;
	if (!test_device(current_drive, buffer,0)) status = FAILURE;

	/* DIREXIST should exist in the current directory */
	sprintf(buffer, "DIREXIST\\%s", device_name);
	if (!test_device('@', buffer,1)) status = FAILURE;            /* drive does not exist, should fail */
	if (!test_device(current_drive, buffer,0)) status = FAILURE;  /* valid drive, should succeed       */

	sprintf(buffer, "BAD_PATH\\%s", device_name);
	if (!test_device('@', buffer,1)) status = FAILURE;
	if (!test_device(current_drive, buffer,1)) status = FAILURE;

	sprintf(buffer, "\\%s", device_name);
	if (!test_device('@', buffer,1)) status = FAILURE;             /* drive does not exist, should fail */
	if (!test_device(current_drive, buffer,0)) status = FAILURE;   /* valid drive, should succeed       */

	sprintf(buffer, "\\BAD_PATH\\%s", device_name);
	if (!test_device('@', buffer,1)) status = FAILURE;
	if (!test_device(current_drive, buffer,1)) status = FAILURE;

	sprintf(buffer, "\\DEV\\%s", device_name);
	if (!test_device('@', buffer,0)) status = FAILURE;
	if (!test_device(current_drive, buffer,0)) status = FAILURE;

	return status;
}

int test_CON()
{
	return do_tests_on("CON");
}

int test_NUL()
{
	return do_tests_on("NUL");
}

int test_COM()
{
	return do_tests_on("COM1");
}

int main(int argc, char *argv[]) 
{
	int error_code = 0;
	if (!test_CON()) error_code |= 0x01;
	if (!test_NUL()) error_code |= 0x02;
	if (!test_COM()) error_code |= 0x04;

	return error_code;
}

/*
  based on tests from andrewbird
  https://github.com/dosemu2/fdpp/pull/42#issuecomment-446309983

	test 1
            fn1 = "*"                                                           
            fe1 = "in"                                                          
            fn2 = "*"                                                           
            fe2 = "out"                                                         
            create the following files "one.in", "two.in", "three.in", "four.in", "five.in", "none.ctl"
	Run test:
             check "one.out, "two.out", "three.out", "four.out", "five.out", "none.ctl" are present

	test 2
            fn1 = "a*"                                                          
            fe1 = "*"                                                           
            fn2 = "b*"                                                          
            fe2 = "out"                                                         
            create "aone.in", "atwo.in", "athree.in", "afour.in", "afive.in", "xnone.ctl"
	Run test:
            check "bone.out", "btwo.out", "bthree.out", "bfour.out", "bfive.out", "xnone.ctl" are present

	test 3
            fn1 = "abc0??"                                                      
            fe1 = "*"                                                           
            fn2 = "???6*"                                                       
            fe2 = "*"                                                           
            create "abc001.txt", "abc002.txt", "abc003.txt", "abc004.txt", "abc005.txt", "abc010.txt", "xbc007.txt"
	Run test:
            check  "abc601.txt", "abc602.txt", "abc603.txt", "abc604.txt", "abc605.txt", "abc610.txt", "xbc007.txt" are present

	test 4
            fn1 = "abc*"                                                        
            fe1 = "htm"                                                         
            fn2 = "*"                                                           
            fe2 = "??"                                                          
            create "abc001.htm", "abc002.htm", "abc003.htm", "abc004.htm", "abc005.htm", "abc010.htm", "xbc007.htm"
Run test:
           check "abc001.ht", "abc002.ht", "abc003.ht", "abc004.ht", "abc005.ht", "abc010.ht", "xbc007.htm" are present
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>

#define	FCB_RENAME 0x17

#define FAILURE 0
#define SUCCESS (!FAILURE)

typedef unsigned char UBYTE;
typedef char BYTE;

#define FNAME_SIZE      8       /* limit on file name           */
#define FEXT_SIZE       3       /* limit on extension           */

#define FDFLT_DRIVE     0       /* default drive                */

/* #pragma pack(1) assumed */
struct fcb {
  UBYTE fcb_drive;              /* Drive number 0=default, 1=A, etc     */
  BYTE old_fname[FNAME_SIZE];   /* File name                    */
  BYTE old_fext[FEXT_SIZE];     /* File name Extension          */
  BYTE padding[5];
  BYTE new_fname[FNAME_SIZE];   /* File name                    */
  BYTE new_fext[FEXT_SIZE];     /* File name Extension          */
  BYTE reserved[9];
};


/* copies s to buffer right padding up to len with spaces */
void set_field(char *buffer, const char *s, int len)
{
	char *buf, *end;
	memset(buffer, ' ', len);
	for (buf=buffer, end = buffer+len; (*s != '\0') && (buf < end); buf++, s++)
	{
		*buf = *s;
	}
}

/* performs fcb rename
   returns handle to open device or -1 on error 
 */
int fcb_rename(const char *old_fname, const char *old_fext,
			   const char *new_fname, const char *new_fext)
{
	union REGS regs;
	struct SREGS sregs;
	struct fcb _fcb;

	printf("FCB rename %s.%s to %s.%s\n", old_fname, old_fext, new_fname, new_fext);

	memset(&_fcb, ' ', sizeof(struct fcb));
	_fcb.fcb_drive = FDFLT_DRIVE;	
	set_field(&(_fcb.old_fname), old_fname, FNAME_SIZE);
	set_field(&(_fcb.old_fext), old_fext, FEXT_SIZE);
	set_field(&(_fcb.new_fname), new_fname, FNAME_SIZE);
	set_field(&(_fcb.new_fext), new_fext, FEXT_SIZE);
	
	
	regs.h.ah = FCB_RENAME;
	regs.h.al = 0;
	regs.x.dx = FP_OFF((void far *)&_fcb);
	sregs.ds = FP_SEG((void far *)&_fcb);
	intdosx(&regs, &regs, &sregs);
	
	if (regs.h.al == 0) return SUCCESS;
	/* al == 0xFF on failure */
	/* printf("fcb rename call returned failure\n"); */
	return FAILURE;
}

/* returns SUCCESS if file exists, else FAILURE */
int file_exists(const char *filename)
{
	FILE *f;
	f = fopen(filename, "r");
	if (f == NULL) return FAILURE;
	fclose(f);
	return SUCCESS;
}

/* returns SUCCESS if file successfully created, else FAILURE */
int create_file(const char *filename)
{
	FILE *f;
	/* printf("creating %s\n", filename); */
	f = fopen(filename, "w");  		
	if (f == NULL) return FAILURE;
	fclose(f);
	if (!file_exists(filename)) return FAILURE;
	return SUCCESS;
}

/* delete test files (either original names or renamed version */
void delete_files_test_1()
{
	remove("one.in");
	remove("two.in");
	remove("three.in");
	remove("four.in");
	remove("five.in");
	remove("none.ctl");
	remove("one.out");
	remove("two.out");
	remove("three.out");
	remove("four.out");
	remove("five.out");
}
void delete_files_test_2()
{
	remove("aone.in");
	remove("atwo.in");
	remove("athree.in");
	remove("afour.in");
	remove("afive.in");
	remove("xnone.ctl");
	remove("bone.out");
	remove("btwo.out");
	remove("bthree.out");
	remove("bfour.out");
	remove("bfive.out");
}
void delete_files_test_3()
{
	remove("abc001.txt");
	remove("abc002.txt");
	remove("abc003.txt");
	remove("abc004.txt");
	remove("abc005.txt");
	remove("abc010.txt");
	remove("xbc007.txt");
	remove("abc601.txt");
	remove("abc602.txt");
	remove("abc603.txt");
	remove("abc604.txt");
	remove("abc605.txt");
	remove("abc610.txt");
}
void delete_files_test_4()
{
	remove("abc001.htm");
	remove("abc002.htm");
	remove("abc003.htm");
	remove("abc004.htm");
	remove("abc005.htm");
	remove("abc010.htm");
	remove("xbc007.htm");
	remove("abc001.ht");
	remove("abc002.ht");
	remove("abc003.ht");
	remove("abc004.ht");
	remove("abc005.ht");
	remove("abc010.ht");
}

/* create needed test files */
int setup_test_1()
{
	/* ensure new files don't exist */
	delete_files_test_1();
	
	if (!create_file("one.in")) return FAILURE;
	if (!create_file("two.in")) return FAILURE;
	if (!create_file("three.in")) return FAILURE;
	if (!create_file("four.in")) return FAILURE;
	if (!create_file("five.in")) return FAILURE;
	if (!create_file("none.ctl")) return FAILURE;
	
	return SUCCESS;
}
int setup_test_2()
{
	/* ensure new files don't exist */
	delete_files_test_2();
	
	if (!create_file("aone.in")) return FAILURE;
	if (!create_file("atwo.in")) return FAILURE;
	if (!create_file("athree.in")) return FAILURE;
	if (!create_file("afour.in")) return FAILURE;
	if (!create_file("afive.in")) return FAILURE;
	if (!create_file("xnone.ctl")) return FAILURE;
	
	return SUCCESS;
}
int setup_test_3()
{
	/* ensure new files don't exist */
	delete_files_test_3();
	
	if (!create_file("abc001.txt")) return FAILURE;
	if (!create_file("abc002.txt")) return FAILURE;
	if (!create_file("abc003.txt")) return FAILURE;
	if (!create_file("abc004.txt")) return FAILURE;
	if (!create_file("abc005.txt")) return FAILURE;
	if (!create_file("abc010.txt")) return FAILURE;
	if (!create_file("xbc007.txt")) return FAILURE;
	if (!create_file("xbc007.htm")) return FAILURE;
	
	return SUCCESS;
}
int setup_test_4()
{
	/* ensure new files don't exist */
	delete_files_test_4();
	
	if (!create_file("abc001.htm")) return FAILURE;
	if (!create_file("abc002.htm")) return FAILURE;
	if (!create_file("abc003.htm")) return FAILURE;
	if (!create_file("abc004.htm")) return FAILURE;
	if (!create_file("abc005.htm")) return FAILURE;
	if (!create_file("abc010.htm")) return FAILURE;
	if (!create_file("xbc007.htm")) return FAILURE;
	
	return SUCCESS;
}

/* attempt fcb rename and verify files changed */
int test_1()
{
	int status = FAILURE;
	setup_test_1();
	
	if (!fcb_rename("*","in", "*","out")) goto failure;
	if (!file_exists("one.out")) goto failure;
	if (!file_exists("two.out")) goto failure;
	if (!file_exists("three.out")) goto failure;
	if (!file_exists("four.out")) goto failure;
	if (!file_exists("five.out")) goto failure;
	if (!file_exists("none.ctl")) goto failure;
	
	status = SUCCESS;	
failure:
	delete_files_test_1();
	return status;
}

int test_2()
{
	int status = FAILURE;
	setup_test_2();
	
	if (!fcb_rename("a*","*", "b*","out")) goto failure;
	if (!file_exists("bone.out")) goto failure;
	if (!file_exists("btwo.out")) goto failure;
	if (!file_exists("bthree.out")) goto failure;
	if (!file_exists("bfour.out")) goto failure;
	if (!file_exists("bfive.out")) goto failure;
	if (!file_exists("xnone.ctl")) goto failure;
	
	status = SUCCESS;	
failure:
	delete_files_test_2();
	return status;
}

int test_3()
{
	int status = FAILURE;
	setup_test_3();
	
	if (!fcb_rename("abc0??","*", "???6*","*")) goto failure;
	if (!file_exists("abc601.txt")) goto failure;
	if (!file_exists("abc602.txt")) goto failure;
	if (!file_exists("abc603.txt")) goto failure;
	if (!file_exists("abc604.txt")) goto failure;
	if (!file_exists("abc605.txt")) goto failure;
	if (!file_exists("abc610.txt")) goto failure;
	if (!file_exists("xbc007.txt")) goto failure;
	
	status = SUCCESS;	
failure:
	delete_files_test_3();
	return status;
}

int test_4()
{
	int status = FAILURE;
	setup_test_4();
	
	if (!fcb_rename("abc*","htm", "*","??")) goto failure;
	if (!file_exists("abc001.ht")) goto failure;
	if (!file_exists("abc002.ht")) goto failure;
	if (!file_exists("abc003.ht")) goto failure;
	if (!file_exists("abc004.ht")) goto failure;
	if (!file_exists("abc005.ht")) goto failure;
	if (!file_exists("abc010.ht")) goto failure;
	if (!file_exists("xbc007.htm")) goto failure;

	status = SUCCESS;	
failure:
	delete_files_test_4();
	return status;
}


int main(int argc, char *argv[]) 
{
	int error_code = 0;

	if (test_1()) { printf("SUCCESS\n"); } else { printf("FAILURE\n");  error_code |= 0x01; }
	if (test_2()) { printf("SUCCESS\n"); } else { printf("FAILURE\n");  error_code |= 0x02; }
	if (test_3()) { printf("SUCCESS\n"); } else { printf("FAILURE\n");  error_code |= 0x04; }
	if (test_4()) { printf("SUCCESS\n"); } else { printf("FAILURE\n");  error_code |= 0x08; }

	return error_code;
}

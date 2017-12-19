/***************************************************************************
*                                                                          *
*  FDKRNCFG.C - FreeDOS Kernel Configuration                               *
*  This is a simple little program that merely displays and/or changes     *
*  the configuration options specified within the CONFIG section of        *
*  the FreeDOS Kernel (if supported)                                       *
*                                                                          *
*  Initially Written by Kenneth J. Davis Oct 11, 2001 (public domain)      *
*  Future versions may contain copyrighted portions, if so the             *
*  copyright holders should be listed after this line.                     *
*  Initial release - public domain                                         *
*                                                                          *
*  merged into SYS by tom ehlert                                                                        *
***************************************************************************/

char VERSION[] = "v1.02";
char PROGRAM[] = "SYS CONFIG";
char KERNEL[] = "KERNEL.SYS";

#include <stdlib.h>
#include <string.h>
#ifndef __GNUC__
#include <fcntl.h>
#endif

#include "portab.h"
/* These definitions deliberately put here instead of
 * #including <stdio.h> to make executable MUCH smaller
 * using [s]printf from prf.c!
 */
extern int VA_CDECL printf(CONST char * fmt, ...);
extern int VA_CDECL sprintf(char * buff, CONST char * fmt, ...);

#if defined(__WATCOMC__)
unsigned _dos_close(int handle);
#define close _dos_close
#define SEEK_SET 0
int open(const char *pathname, int flags, ...);
int read(int fd, void *buf, unsigned count);
int write(int fd, const void *buf, unsigned count);
int stat(const char *file_name, struct stat *buf);
unsigned long lseek(int fildes, unsigned long offset, int whence);
#pragma aux lseek =  \
      "mov ah, 0x42" \
      "int 0x21"     \
      parm [bx] [dx cx] [ax] \
      value [dx ax];

#elif defined(__GNUC__)
#include <unistd.h>
#include <fcntl.h>
#define memicmp strncasecmp
#define O_BINARY 0
#else
#include <io.h>
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
/* #include <stdio.h> */
#endif

#define FAR far
#include "kconfig.h"

KernelConfig cfg; /* static memory zeroed automatically */

typedef unsigned char byte;
typedef signed char sbyte;
typedef unsigned short word;
typedef signed short sword;
typedef unsigned long dword;
typedef signed long sdword;


/* Displays command line syntax */
void showUsage(void)
{
  printf("Usage: \n"
         "  %s \n"
         "  %s [/help | /?]\n"
         "  %s [ [drive:][path]%s] [option=value ...] \n",
         PROGRAM, PROGRAM, PROGRAM, KERNEL);
  printf("\n");
  printf("  If no options are given, the current values are shown.\n");
  printf("  /help or /? displays this usage information.\n"
         "   [drive:][path]KERNEL.SYS specifies the kernel file to\n"
         "      modify, if not given defaults to %s\n", KERNEL);
  printf("\n");
  printf
      ("  option=value ... specifies one or more options and the values\n"
       "      to set each to.  If an option is given multiple times,\n"
       "      the value set will be the rightmost one.\n");
  printf("  Current Options are: DLASORT=0|1, SHOWDRIVEASSIGNMENT=0|1\n"
         "                       SKIPCONFIGSECONDS=#, FORCELBA=0|1\n"
         "                       GLOBALENABLELBASUPPORT=0|1\n"
         "                       BootHarddiskSeconds=0|seconds to wait\n");
}

/* simply reads in current configuration values, exiting program
   with an error message and error code unable to, otherwise
   cfg & kfile are valid on return.
*/

/* Reads in the current kernel configuration settings,
   return 0 on success, nonzero on error.  If there was
   an actual error the return value is positive, if there
   were no errors, but the CONFIG section was not found
   then a negative value is returned.  cfg is only altered
   if the return value is 0 (ie successfully found and
   read in the config section).  The position of the file
   pointer on input does not matter, the file position
   upon return may be anywhere.  The memory allocated for
   cfg should be freed to prevent memory leakage (it should
   not point to allocated memory on entry, as that memory
   will not be used, and will likely not be freed as a result).
*/
int readConfigSettings(int kfile, char *kfilename, KernelConfig * cfg)
{
  /* Seek to start of kernel file */
  if (lseek(kfile, 2, SEEK_SET) != 2)
    printf("can't seek to offset 2\n"), exit(1);

  if (read(kfile, cfg, sizeof(KernelConfig)) != sizeof(KernelConfig))
    printf("can't read %u bytes\n", sizeof(KernelConfig)), exit(1);

  if (memcmp(cfg->CONFIG, "CONFIG", 6) != 0)
  {
    printf("Error: no CONFIG section found in kernel file <%s>\n",
           kfilename);
    printf("Only FreeDOS kernels after 2025 contain a CONFIG section!\n");
    exit(1);
  }

  /* check if config settings old UPX header and adjust */
  if (cfg->ConfigSize == 19)
    cfg->ConfigSize = 14;  /* ignore 'nused87654321' */

  return 1;
}

/* Writes config values out to file.
   Returns 0 on success, nonzero on error.
*/
int writeConfigSettings(int kfile, KernelConfig * cfg)
{
  /* Seek to CONFIG section at start of options of kernel file */
  if (lseek(kfile, 2, SEEK_SET) != 2)
    return 1;

  /* Write just the config option information out */
  if (write(kfile, cfg, sizeof(KernelConfig)) != sizeof(KernelConfig))
    return 1;

  /* successfully wrote out kernel config data */
  return 0;
}

/* Displays kernel configuration information */
void displayConfigSettings(KernelConfig * cfg)
{
  /* print known options and current value - only if available */

  /* show kernel version if available, read only, no option to modify */
  if (cfg->ConfigSize >= 20)
  {
    printf
        ("%s kernel %s (build %d.%d OEM:%02X)\n", 
        (cfg->Version_OemID == 0xFD)?"FreeDOS":"DOS-C",
        cfg->Version_Release?"SVN":"Release",
        cfg->Version_Major,
        cfg->Version_Revision,
        cfg->Version_OemID
        );
  }

  if (cfg->ConfigSize >= 1)
  {
    printf
        ("DLASORT=0x%02X              Sort disks by drive order:  *0=no, 1=yes\n",
         cfg->DLASortByDriveNo);
  }

  if (cfg->ConfigSize >= 2)
  {
    printf
        ("SHOWDRIVEASSIGNMENT=0x%02X  Show how drives assigned:   *1=yes 0=no\n",
         cfg->InitDiskShowDriveAssignment);
  }

  if (cfg->ConfigSize >= 3)
  {
    printf
        ("SKIPCONFIGSECONDS=%-3d     time to wait for F5/F8:     *2 sec (skip < 0)\n",
         cfg->SkipConfigSeconds);
  }

  if (cfg->ConfigSize >= 4)
  {
    printf
        ("FORCELBA=0x%02X             Always use LBA if possible: *0=no, 1=yes\n",
         cfg->ForceLBA);
  }

  if (cfg->ConfigSize >= 5)
  {
    printf
        ("GLOBALENABLELBASUPPORT=0x%02X Enable LBA support:       *1=yes, 0=no\n",
         cfg->GlobalEnableLBAsupport);
  }

  if (cfg->ConfigSize >= 6)
  {
    printf
        ("BootHarddiskSeconds=%d :      *0=no else seconds to wait for key\n",
         cfg->BootHarddiskSeconds);
  }

#if 0                           /* we assume that SYS is as current as the kernel */

  /* Print value any options added that are unknown as hex dump */
  if (cfg->configHdr.configSize > sizeof(ConfigData))
  {
    printf("Additional options are available, they are not currently\n"
           "supported by this tool.  The current extra values are (in Hex):\n");
    for (i = 0; i < (cfg->configSize - sizeof(ConfigData)); i++)
    {
      if ((i % 32) == 0)
        printf("\n");
      else if ((i % 4) == 0)
        printf(" ");
      printf("%02X", (unsigned int)cfg->extra[i]);
    }
    printf("\n");
  }
#endif
  printf("\n");
}

/* Note: The setXXXOption functions will set the config option of
   type XXX to the value given.  It will display a warning, but
   allow probably invalid values to be used (cause I believe in
   letting the user do what they want, not what we guess they mean).
   Additionally, we only indicate a change if a new value is used,
   to force changes written even if same value is used, use same
   option twice, first with a different value & second time with
   (same) value desired.  kjd
*/

/* Sets the given location to an unsigned byte value if different,
   displays warning if values exceeds max
*/
void setByteOption(byte * option, char *value, word max, int *updated,
                   char *name)
{
  unsigned long optionValue;

  /*  optionValue = atoi(value); Use strtoul instead of atoi/atol as it detect base (0xFF & 255) */
  optionValue = strtoul(value, NULL, 0);

  if (optionValue > 255)
  {
    printf("Warning: Option %s: Value <0x%02lX> will be truncated!\n",
           name, optionValue);
  }
  if ((byte) optionValue > max)
  {
    printf("Warning: Option %s: Value <0x%02X> may be invalid!\n",
           name, (unsigned int)((byte) optionValue));
  }
  /* Don't bother updating if same value */
  if ((byte) optionValue != *option)
  {
    *option = (byte) optionValue;
    *updated = 1;
  }
}

/* Sets the given location to a signed byte value if different,
   displays warning if values exceeds max or is less than min
*/
void setSByteOption(sbyte * option, char *value, sword min, sword max,
                    int *updated, char *name)
{
  signed long optionValue;

  /*  optionValue = atoi(value); Use strtol instead of atoi/atol as it detects base */
  optionValue = strtol(value, NULL, 0);

  if ((optionValue < -128) || (optionValue > 127))
  {
    printf("Warning: Option %s: Value <0x%02lX> will be truncated!\n",
           name, optionValue);
  }
  if (((sbyte) optionValue > max) || ((sbyte) optionValue < min))
  {
    printf("Warning: Option %s: Value <0x%02X> may be invalid!\n",
           name, (signed int)((byte) optionValue));
  }
  /* Don't bother updating if same value */
  if ((sbyte) optionValue != *option)
  {
    *option = (sbyte) optionValue;
    *updated = 1;
  }
}

#if 0                           /* disable until there are (un)signed word configuration values */
/* Sets the given location to an unsigned word value if different,
   displays warning if values exceeds max
*/
void setWordOption(word * option, char *value, dword max, int *updated,
                   char *name)
{
  unsigned long optionValue;

  /*  optionValue = atol(value); Use strtoul instead of atoi/atol as it allows 0xFF and 255 */
  optionValue = strtoul(value, NULL, 0);

  if (optionValue > 65535)
  {
    printf("Warning: Option %s: Value <0x%02lX> will be truncated!\n",
           name, optionValue);
  }
  if ((word) optionValue > max)
  {
    printf("Warning: Option %s: Value <0x%02X> may be invalid!\n",
           name, (unsigned int)optionValue);
  }
  /* Don't bother updating if same value */
  if ((word) optionValue != *option)
  {
    *option = (word) optionValue;
    *updated = 1;
  }
}

/* Sets the given location to a signed byte value if different,
   displays warning if values exceeds max or is less than min
*/
void setSWordOption(sword * option, char *value, sdword min, sdword max,
                    int *updated, char *name)
{
  signed long optionValue;

  /*  optionValue = atol(value); Use strtol instead of atoi/atol as it allows 0xFF and 255 */
  optionValue = strtol(value, NULL, 0);

  if ((optionValue < -32768) || (optionValue > 32767))
  {
    printf("Warning: Option %s: Value <0x%02lX> will be truncated!\n",
           name, optionValue);
  }

  if (((sword) optionValue > max) || ((sword) optionValue < min))
  {
    printf("Warning: Option %s: Value <0x%02X> may be invalid!\n",
           name, (signed int)optionValue);
  }
  /* Don't bother updating if same value */
  if ((sword) optionValue != *option)
  {
    *option = (sword) optionValue;
    *updated = 1;
  }
}
#endif

/* Main, processes command line options and calls above
   functions as required.
*/
int FDKrnConfigMain(int argc, char **argv)
{
  char *kfilename = KERNEL;
  int kfile;
  int updates = 0;              /* flag used to indicate if we need to update kernel */
  int readonly = 0;             /* flag indicates kernel was opened read-only */
  int argstart, i;
  char *cptr;
  char *argptr;

  printf("FreeDOS Kernel Configuration %s\n", VERSION);

  /* 1st go through and just process arguments (help/filename/etc) */
  for (i = 1; i < argc; i++)
  {
    argptr = argv[i];

    /* is it an argument or an option specifier */
    if (argptr[0] == '-' || argptr[0] == '/')
    {
      switch (argptr[1])
      {
        case 'H':
        case 'h':
        case '?':
          showUsage();
          exit(0);

        default:
          printf("Invalid argument found <%s>.\nUse %s /help for usage.\n",
                 argptr, PROGRAM);
          exit(1);
      }
    }
    else if (memicmp(argptr, "CONFIG", 6) == 0)
    {
      /* ignore */
    }
  }

  argstart = 2;

  argptr = argv[argstart];

#if 0                           /* No arguments is acceptable, just displays current settings using default kernel file */
  if (argptr == 0)
  {
    showUsage();
    exit(1);
  }
#endif

  /* the first argument may be the kernel name */
  if ((argstart < argc) && (strchr(argptr, '=') == NULL))
  {
    kfilename = argptr;
    argstart++;
  }

  kfile = open(kfilename, O_RDWR | O_BINARY);

  if (kfile < 0)
  {
    /* attempt to open read only to allow viewing options */
    kfile = open(kfilename, O_RDONLY  | O_BINARY);
    readonly = 1;

    if (kfile < 0)
      printf("Error: unable to open kernel file <%s>\n", kfilename), exit(1);
  }

  /* now that we know the filename (default or given) get config info */
  readConfigSettings(kfile, kfilename, &cfg);

  for (i = argstart; i < argc; i++)
  {
    argptr = argv[i];

    if ((cptr = strchr(argptr, '=')) == NULL)
      goto illegal_arg;

    /* split argptr into 2 pieces and make cptr point to 2nd one */
    *cptr = '\0';
    cptr++;

    /* allow 3 valid characters */
    if (memicmp(argptr, "DLASORT", 3) == 0)
    {
      setByteOption(&(cfg.DLASortByDriveNo), cptr, 1, &updates, "DLASORT");
    }
    else if (memicmp(argptr, "SHOWDRIVEASSIGNMENT", 3) == 0)
    {
      setByteOption(&(cfg.InitDiskShowDriveAssignment),
                    cptr, 1, &updates, "SHOWDRIVEASSIGNMENT");
    }
    else if (memicmp(argptr, "SKIPCONFIGSECONDS", 3) == 0)
    {
      setSByteOption(&(cfg.SkipConfigSeconds),
                     cptr, -128, 127, &updates, "SKIPCONFIGSECONDS");
    }
    else if (memicmp(argptr, "FORCELBA", 3) == 0)
    {
      setByteOption(&(cfg.ForceLBA), cptr, 1, &updates, "FORCELBA");
    }
    else if (memicmp(argptr, "GLOBALENABLELBASUPPORT", 3) == 0)
    {
      setByteOption(&(cfg.GlobalEnableLBAsupport),
                    cptr, 1, &updates, "GLOBALENABLELBASUPPORT");
    }
    else if (memicmp(argptr, "BootHarddiskSeconds", 3) == 0)
    {
      setSByteOption(&(cfg.BootHarddiskSeconds),
                     cptr, 0, 127, &updates, "BootHarddiskSeconds");
    }
    else
    {
    illegal_arg:
      printf("Unknown option found <%s>.\nUse %s /help for usage.\n",
             argptr, PROGRAM);
      exit(1);
    }
  }

  /* warn user if attempt to modify read-only file */
  if (updates && readonly)
  {
      printf("Kernel %s opened read-only, changes ignored!\n", kfilename);
      /* reload current settings, ignore newly requested ones */
      readConfigSettings(kfile, kfilename, &cfg);
  }

  /* write out new config values if modified */
  if (updates && !readonly)
  {
    /* update it */
    if (writeConfigSettings(kfile, &cfg))
    {
      printf("Error: Unable to write configuration changes to kernel!\n");
      printf("       <%s>\n", kfilename);
      close(kfile);
      exit(1);
    }

    /* display new settings  */
    printf("\nUpdated Kernel settings.\n");
  }
  else
    printf("\nCurrent Kernel settings.\n");

  /* display current settings  */
  displayConfigSettings(&cfg);

  /* and done */
  close(kfile);

  return 0;
}

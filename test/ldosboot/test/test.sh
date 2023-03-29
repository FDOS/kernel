#! /bin/bash

# Usage of the works is permitted provided that this
# instrument is retained with the works, so that any entity
# that uses the works is notified of this instrument.
#
# DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

if [[ "$1" != selfcall ]]
then
  setsid -w "$0" selfcall "$@"
  exit $?
fi
shift

. cfg.sh

if [ -n "$LMACROS_DIR" ]; then {
  options_i_lmacros=-I"${LMACROS_DIR%/}"/
} fi

if [ -n "$LDOSBOOT_DIR" ]; then {
  options_i_ldosboot=-I"${LDOSBOOT_DIR%/}"/
} fi

if [ -n "$SCANPTAB_DIR" ]; then {
  options_i_scanptab=-I"${SCANPTAB_DIR%/}"/
} fi

if [ -n "$BOOTIMG_DIR" ]; then {
  options_i_bootimg=-I"${BOOTIMG_DIR%/}"/
} fi

if [ -n "$LDEBUG_DIR" ]; then {
  options_i_ldebug=-I"${LDEBUG_DIR%/}"/
} fi

if [ -n "$LDOSMBR_DIR" ]; then {
  options_i_ldosmbr=-I"${LDOSMBR_DIR%/}"/
} fi

if [ -n "$INSTSECT_DIR" ]; then {
  options_i_instsect=-I"${INSTSECT_DIR%/}"/
} fi

direct=0
options_dosemu_direct="-A"
bpe=12
spc=1
spi=2880
nr=224
pitype=""
chsheads=8
chssectors=8

((debug)) && echo DEFAULT_MACHINE="$DEFAULT_MACHINE" parameters="$@"
machine="$DEFAULT_MACHINE"
if [[ "$1" == dosemu ]]
then
  machine="$1"
  shift
elif [[ "$1" == qemu ]]
then
  machine="$1"
  shift
fi

driveletters=( {C..Z} )
unitletters=( {a..z} )
dosemu=0
qemu=0
if [[ "$machine" == dosemu ]]
then
  dosemu=1
  if [ -z "$dosemu_drives" ] || [ "$dosemu_drives" == test ]
  then
    # Depending on the dosemu2 configuration, dosemu may
    #  set up 2, 3, or 4 system drives. To support all
    #  these cases, the variable dosemu_drives may now
    #  be set to indicate the amount of drives.
    # If it is not set or set to "test" we do a bit of
    #  magic detection: Run dosemu -E lredir and count
    #  the amount of redirections displayed. Also check
    #  that all of the redirections are on the expected
    #  drives, starting with C:.
    # We assume that each of the system drives corresponds
    #  to its own ROM-BIOS unit.
    # Discussed here: https://github.com/dosemu2/dosemu2/discussions/1456
    driveslist="$("$DOSEMU" -dumb -quiet -E lredir -te 2> /dev/null < /dev/null |
      sed -re '/^about to execute|^current drive redirections/Id' )"
    dosemu_drives="$(echo "$driveslist" | wc -l)"
    for checkdrivenumber in $(seq 0 "$((dosemu_drives - 1))")
    do
      driveletter="${driveletters[checkdrivenumber]}"
      if ! echo "$driveslist" | grep -Eiq "^$driveletter:"
      then
        echo "Error: Drive $driveletter: not found in lredir list" >&2
        exit 1
      fi
    done
  fi
  ((debug)) && echo "Drive ${driveletters[dosemu_drives]}:"
elif [[ "$machine" == qemu ]]
then
  qemu=1
  dosemu_drives=0
else
  echo "Error: invalid machine \"$machine\" selected" >&2
  exit 1
fi

if [[ "$1" == hdimage ]]
then
  shift
  options_hdimage="-D_MBR"
  if ((dosemu))
  then
    options_hdimage="$options_hdimage -D_MBR_DOSEMU_IMAGE_HEADER"
  fi
  dosemu_category="disk"
  dosemu_device="hdimage"
  dosemu_drive="${driveletters[dosemu_drives]}:"
  qemu_switch="-hda"
  name="hdimage"
  bootfile=""
  unit="$(printf "%03Xh" "$((0x80 + $dosemu_drives))")"
  options_ldebug_unit="hd${unitletters[dosemu_drives]}"
  diskette=0
  if [[ "$1" == direct ]]
  then
    shift
    # output of the following command is either:
    # 1. "CPU set to 486", followed by the sign ons for dosemu, FreeDOS,
    #     and config.sys and autoexec.bat output, then executing "exitemu"
    #     (if -C4 switch is not supported, parsed as -C -4).
    # 2. Only the dosemu sign on with the error message as follows:
    #     "ERROR: Drive G not defined, can't boot!" (execution aborted).
    if ((dosemu)) && "$DOSEMU" -dumb -E exitemu -C4 < /dev/null 2>&1 | grep "CPU set to 486" > /dev/null
    then
      echo "Error: dosemu lacks support for -C4 switch" >&2
      exit 1
    fi
    options_dosemu_direct="-C$dosemu_drives"
    unit=80h
    direct=1
  fi
  if ((qemu))
  then
    unit=80h
    options_ldebug_unit="hda"
  fi
  ((debug)) && echo "unit=\"$unit\""
  while true
  do
    if [[ "$1" == bpe && -n "$2" ]]
    then
      ((bpe="$2"))
      if (($?)) || [[ "$bpe" != 32 && "$bpe" != 16 && "$bpe" != 12 ]]
      then
        echo "Error: Invalid bpe \"$2\" given, expected 12, 16, 32" >&2
        exit 1
      fi
      shift
      shift
    elif [[ "$1" == spc && -n "$2" ]]
    then
      ((spc="($2)"))
      (($?)) && exit $?
      shift
      shift
    elif [[ "$1" == mib && -n "$2" ]]
    then
      ((spi="(($2) * 1024 * 2)"))
      (($?)) && exit $?
      shift
      shift
    elif [[ "$1" == spi && -n "$2" ]]
    then
      ((spi="($2)"))
      (($?)) && exit $?
      shift
      shift
    elif [[ "$1" == nr && -n "$2" ]]
    then
      ((nr="($2)"))
      (($?)) && exit $?
      shift
      shift
    elif [[ "$1" == pitype && -n "$2" ]]
    then
      pitype="$2"
      shift
      shift
    elif [[ "$1" == aligndata ]]
    then
      options_hdimage="$options_hdimage -D_ALIGNDATA"
      shift
    elif [[ "$1" == chsheads && -n "$2" ]]
    then
      ((chsheads="($2)"))
      (($?)) && exit $?
      shift
      shift
    elif [[ "$1" == chssectors && -n "$2" ]]
    then
      ((chssectors="($2)"))
      (($?)) && exit $?
      shift
      shift
    else
      break
    fi
  done
  if ((qemu && chsheads > 16))
  then
    echo "Warning: qemu autodetection requires CHS heads <= 16" >&2
  fi
  if ((dosemu))
  then
    options_mcopy_offset="@@$((16 + chsheads * chssectors))s"
  else
    options_mcopy_offset="@@$((chsheads * chssectors))s"
  fi
  options_hdimage="$options_hdimage -D_CHS_HEADS=$chsheads -D_CHS_SECTORS=$chssectors"
elif [[ "$1" == diskette ]]
then
  shift
  options_hdimage=""
  dosemu_category="floppy"
  dosemu_device="device"
  dosemu_drive="A:"
  qemu_switch="-fdb"
  name="diskette"
  bootfile="boot${bpe}tw.bin"
  unit=01h
  options_ldebug_unit="fdb"
  options_mcopy_offset=""
  diskette=1
  if [[ "$1" == direct ]]
  then
    shift
    qemu_switch="-fda"
    unit=00h
    direct=1
  fi
else
  echo "Error: Invalid disk type \"$1\" specified , must be hdimage or diskette" >&2
  exit 1
fi

if [[ "$bpe" == 32 ]]
then
  if [[ -z "$pitype" ]]
  then
    pitype=ptFAT32
  fi
  bootname=boot32
else
  if [[ -z "$pitype" ]]
  then
    if [[ "$bpe" == 16 ]]
    then
      pitype=ptFAT16
    else
      pitype=ptFAT12
    fi
  fi
  bootname=boot
fi

echo -ne 'failure\r\n' > result.txt

TMPINC=""
if command -v "$MKTMPINC" &> /dev/null
then
	TMPINC="-D_TMPINC"
fi
if [[ -n "$TMPINC" ]]
then
	"$MKTMPINC" "${LDOSBOOT_DIR%/}"/$bootname.asm > /dev/null
fi
 "$NASM" "${LDOSBOOT_DIR%/}"/$bootname.asm -w-user \
  $TMPINC \
  -D_LOAD_NAME="'TESTWRIT'" -D_LOAD_EXT="'SYS'" -D_FAT$bpe \
  -D_UNIT=$unit \
  "$@" \
  "$options_i_lmacros" \
  -D_MAP=boot${bpe}tw.map -l boot${bpe}tw.lst -o boot${bpe}tw.bin &&
 "$NASM" "${LDOSBOOT_DIR%/}"/testwrit.asm \
  "$options_i_lmacros" \
  -o testwrit.bin -l testwrit.lst &&
 "$NASM" "${LDOSBOOT_DIR%/}"/iniload.asm -w-user \
  "$options_i_ldosboot" \
  "$options_i_lmacros" \
  "$options_i_scanptab" \
  -D_PADDING='(48 * 1024)' \
  -D_PAYLOAD_FILE="'testwrit.bin'" -o testwrit.sys -l testwrin.lst \
  -D_INILOAD_SIGNATURE='"TW"' &&
 "$NASM" "${BOOTIMG_DIR%/}"/bootimg.asm \
  -D_WARN_DEFAULT_OFF=1 \
  -D_WARN_TOOMANYFAT=0 -D_WARN_ALIGNDATA=0 \
  $options_hdimage -D_MBR_PART_TYPE="$pitype" \
  -D_BPE="$bpe" -D_SPC="$spc" -D_SPI="$spi" \
  -D_SPF="$(( (spi / spc * bpe / 8 + 511) / 512 ))" \
  -D_NUMROOT="$nr" \
  -D_MAP=$name.map -o $name.img -l $name.lst \
  -D_PAYLOADFILE="testwrit.sys,result.txt,::chdir,dir" \
  -D_BOOTFILE="'$bootfile'" \
  -D_UNIT=$unit \
  "$@" \
  -I ./ \
  "$options_i_lmacros" \
  "$options_i_bootimg"
(($?)) && exit $?
if [[ -n "$TMPINC" ]]
then
	rm -f *.tmp
fi

pgid="$(ps -o pgid= $$)"
function handle_timeout_process() {
  stty sane
  ((debug)) && ps -e -o pgid=,comm=,pid= | grep -E "^\s*$pgid "
  pidlist="$(ps -e -o pgid=,comm=,pid= |
    grep -E "^\s*$pgid " |
    grep -Ev " (bash|test.sh|ps|grep) ")"
  pidlist="$(echo "$pidlist" |
    sed -re 's/^\s+//g' |
    tr -s " " | cut -d" " -f 3)"
  if [[ -n "$pidlist" ]]
  then
    ((debug)) && ps $pidlist
    kill $pidlist
  fi
}

if ((! diskette))
then
  if ((dosemu))
  then
    seekmbr=8192
  else
    seekmbr=0
  fi
  "$NASM" "${LDOSMBR_DIR%/}"/oldmbr.asm -w-user \
  "$options_i_ldosmbr" \
  "$options_i_lmacros" \
  -o oldmbr.bin &&
  dd if=oldmbr.bin of=hdimage.img seek="$seekmbr" bs=1 count=440 conv=notrunc status=none
  (($?)) && exit $?

  "$NASM" "${INSTSECT_DIR%/}"/instsect.asm \
   -w-macro-params-legacy \
   -I ./ \
   "$options_i_instsect" \
   "$options_i_lmacros" \
   -D_FAT12=0 -D_FAT16=0 -D_FAT32=0 -D_FAT$bpe=1 \
   -D_PAYLOAD_FAT$bpe="'boot${bpe}tw.bin'" \
   -o inst${bpe}tw.com -l inst${bpe}tw.lst -D_MAP=inst${bpe}tw.map
  (($?)) && exit $?
  if ((dosemu))
  then
    timeout --foreground 10 "$DOSEMU" \
     -I "$dosemu_category { $dosemu_device $name.img }" \
     -dumb -quiet > result.log 2>&1 -K "$PWD" \
     -E "inst${bpe}tw.com $dosemu_drive" < /dev/null
    rc=$?
    handle_timeout_process
    if ((rc))
    then
      if ((rc == 124))
      then
        echo "Error: instsect run timed out" >&2
      fi
      cat result.log
      exit 1
    fi
    # && "$DOSEMU" -I "..." -dumb -K "$PWD" -E dirg.bat;
  elif ((qemu))
  then
    cp -aL "$BOOT_KERNEL" "${BOOT_KERNEL##*/}"
    cp -aL "$BOOT_COMMAND" "${BOOT_COMMAND##*/}"
    echo -ne "@echo off\r\ninst${bpe}tw.com C:\r\nquit.com\r\n" > autoexec.bat
if [[ -n "$TMPINC" ]]
then
	"$MKTMPINC" "${LDOSBOOT_DIR%/}"/boot.asm > /dev/null
fi
    "$NASM" quit.asm \
     "$options_i_lmacros" \
     -o quit.com &&
    "$NASM" "${LDOSBOOT_DIR%/}"/boot.asm -w-user \
     "$options_i_lmacros" \
     $TMPINC \
     -D_COMPAT_"$BOOT_PROTOCOL"=1 \
     -D_LBA=0 -D_USE_PART_INFO=0 -D_QUERY_GEOMETRY=0 \
    $BOOT_OPTIONS \
     -D_MAP=bootinst.map -l bootinst.lst -o bootinst.bin &&
    "$NASM" "${BOOTIMG_DIR%/}"/bootimg.asm \
     -I ./ \
     "$options_i_ldebug" \
     "$options_i_bootimg" \
     "$options_i_lmacros" \
     -o diskinst.img -l diskinst.lst \
     -D_PAYLOADFILE="${BOOT_KERNEL##*/},${BOOT_COMMAND##*/},autoexec.bat,inst${bpe}tw.com,quit.com" \
     -D_BOOTFILE="'bootinst.bin'"
    (($?)) && exit $?
if [[ -n "$TMPINC" ]]
then
	rm -f *.tmp
fi
    timeout --foreground 10 "$QEMU" -fda diskinst.img "$qemu_switch" "$name".img -boot order=a -display none 2> /dev/null
    rc=$?
    handle_timeout_process
    if ((rc == 124))
    then
      echo "Error: instsect run timed out" >&2
      exit 1
    fi
  fi
fi

if ((! direct))
then
if [[ -n "$TMPINC" ]]
then
	"$MKTMPINC" "${LDOSBOOT_DIR%/}"/boot.asm > /dev/null
fi
 "$NASM" "${LDOSBOOT_DIR%/}"/boot.asm -w-user \
  "$options_i_lmacros" \
  $TMPINC \
  -D_LOAD_NAME="'LDEBUG'" -D_LOAD_EXT="'COM'" \
  -D_MAP=boot12db.map -l boot12db.lst -o boot12db.bin &&
 "$NASM" "${BOOTIMG_DIR%/}"/bootimg.asm \
  -I ./ \
  "$options_i_ldebug" \
  "$options_i_bootimg" \
  "$options_i_lmacros" \
  -o diskldbg.img -l diskldbg.lst \
  -D_PAYLOADFILE="ldebug.com" -D_BOOTFILE="'boot12db.bin'"
 (($?)) && exit $?
if [[ -n "$TMPINC" ]]
then
	rm -f *.tmp
fi

  if ((dosemu))
  then
    timeout --foreground 10 "$DOSEMU" -input \
     "$(echo -ne "boot $options_ldebug_unit"'\rif (rc) then boot quit\rq\r')" \
     -I "floppy { device diskldbg.img }" \
     -I "$dosemu_category { $dosemu_device $name.img }" \
     -A -dumb 2> result.err > result.log < /dev/null
  elif ((qemu))
  then
    ( (sleep 2; "$SENDKEYS" "boot $options_ldebug_unit<ret>if (rc) then boot quit<ret>q<ret>" | socat - UNIX-CONNECT:qemu-monitor > /dev/null ) & > /dev/null)
    timeout --foreground 10 "$QEMU" -fda diskldbg.img "$qemu_switch" "$name".img -boot order=a -monitor unix:qemu-monitor,server,nowait -display none 2> /dev/null
  fi
else
  if ((dosemu))
  then
    timeout --foreground 10 "$DOSEMU" \
     -I "$dosemu_category { $dosemu_device $name.img }" \
     "$options_dosemu_direct" -dumb 2> result.err > result.log < /dev/null
  elif ((qemu))
  then
    timeout --foreground 10 "$QEMU" "$qemu_switch" "$name".img -display none 2> /dev/null
  fi
fi

rc=$?
handle_timeout_process
if ((rc == 124))
then
  echo "${testrunname}timeout"
fi
if [[ "$(mtype -t -i $name.img$options_mcopy_offset ::RESULT.TXT 2> /dev/null)" == success ]]
then
  echo "${testrunname}success"
elif ((dosemu))
then
  echo "${testrunname}failure, log contains:"
  if [ ! -s result.log ]
  then
    echo "Result log file empty, dosemu error?"
  fi
  cat result.log | perl -e '
    my %errorlookup = (
      V => "Check value mismatch",
      F => "File not found",
      E => "Not enough file data",
      R => "Disk read error",
      B => "Bad chain / bad FS",
      M => "Out of memory",
      I => "FSIBOOT error",
      S => "Fix sector size mismatch",
      C => "Fix cluster size mismatch",
      N => "Non-FAT load needs larger cluster size",
    );
    my $empty = 1;
    while (<>) {
      next if $empty and /^\s*$/;
      next if /^(dosemu2 2|Configured: |Please test against)/;
      next if /^(Get the latest code|Submit Bugs via|Ask for help in)/;
      next if /^(This program comes with|This is free software,)/;
      $empty = 0;
      if (/^([A-Z])\x7/ and defined $errorlookup{$1}) {
	s/^(.).//;
	print "lDOS boot error condition letter \"$1\" = $errorlookup{$1}\n";
	s/[\x7\n]//g;
	s/^\s*$//;
	next if /^(dosemu2 2|Configured: |Please test against)/;
	next if /^(Get the latest code|Submit Bugs via|Ask for help in)/;
	next if /^(This program comes with|This is free software,)/;
	print;
	print "\n" unless /^$/;
      } else {
	s/[\x7\n]//g;
	print;
	print "\n";
      };
    };'
else
  echo "${testrunname}failure"
fi

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

direct=1
bpe=12
spc=1
spi=2880
nr=224
pitype=""

machine="qemu"
qemu=1

  options_hdimage=""
  options_mcopy_offset=""
  diskette=1
    qemu_switch="-fda"
    unit=00h
    direct=1
  if [[ -z "$pitype" ]]
  then
    if [[ "$bpe" == 16 ]]
    then
      pitype=ptFAT16
    else
      pitype=ptFAT12
    fi
  fi

echo -ne 'shell=test.com\r\n' > fdconfig.sys
echo -ne 'failure\r\n' > result.txt
if [[ -z "$1" || -z "$2" || -z "$3" ]]
then
  echo Error, missing parameters. 1=KERNELPATHNAME, 2=DISKETTE, 3=BOOT
  exit 1
fi
KERNELPATHNAME="$1"
DISKETTE="$2"
BOOT="$3"
testrunname="$4"
shift
shift
shift
shift

"$NASM" "test.asm" \
  "$options_i_lmacros" \
  -o test.com &&
"$NASM" "${LDOSBOOT_DIR%/}"/boot.asm -w-user \
  "$options_i_lmacros" \
  -D_COMPAT_FREEDOS=1 \
  -D_LBA=0 -D_USE_PART_INFO=0 -D_QUERY_GEOMETRY=0 \
  -D_MAP=boot.map \
  -D_FAT$bpe -D_UNIT=$unit \
  "$@" \
  -l "$BOOT".lst \
  -o "$BOOT".bin &&
"$NASM" "${BOOTIMG_DIR%/}"/bootimg.asm \
  -I ./ \
  "$options_i_bootimg" \
  "$options_i_lmacros" \
  -D_PAYLOADFILE="::rename,'$KERNELPATHNAME',KERNEL.SYS,fdconfig.sys,test.com,result.txt" \
  -D_BOOTPATCHFILE="'$BOOT.bin'" \
  -D_WARN_DEFAULT_OFF=1 \
  -D_WARN_TOOMANYFAT=0 -D_WARN_ALIGNDATA=0 \
  $options_hdimage -D_MBR_PART_TYPE="$pitype" \
  -D_BPE="$bpe" -D_SPC="$spc" -D_SPI="$spi" \
  -D_SPF="$(( (spi / spc * bpe / 8 + 511) / 512 ))" \
  -D_NUMROOT="$nr" \
  -o "$DISKETTE".img -l "$DISKETTE".lst \
  -D_UNIT=$unit \
  "$@"
(($?)) && exit $?

pgid="$(ps -o pgid= $$)"
function handle_timeout_process() {
  stty sane 2> /dev/null > /dev/null
  # The stty sane command in the CI environment shows:
  #
  # 'standard input': Inappropriate ioctl for device
  #
  #  It is probably safe to ignore.
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
timeout --foreground 10 "$QEMU" "$qemu_switch" "$DISKETTE".img -display none 2> /dev/null

rc=$?
handle_timeout_process
if ((rc == 124))
then
  echo "${testrunname}timeout"
  exit 124
fi
if [[ "$(mtype -t -i $DISKETTE.img$options_mcopy_offset ::RESULT.TXT 2> /dev/null)" == success ]]
then
  echo "${testrunname}success"
  exit 0
else
  echo "${testrunname}failure"
  exit 1
fi

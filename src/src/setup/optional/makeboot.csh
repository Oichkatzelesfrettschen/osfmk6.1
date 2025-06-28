#!/bin/csh -f
#
# run makeboot.csh command from under ./obj/<target_machine>/mach_kernel/<CONFIG>
# to produce a bootable image, mach.boot, incorporating the
# bootstrap and kernel

if !(-e mach_kernel) then
        echo "you must have a built kernel under ./obj/<target_machine>/mach_kernel/<CONFIG>"
        exit 1
endif
if !(-e ../../mach_services/cmds/makeboot/makeboot) then
	echo "you must build makeboot under ./src/mach_services/cmds"
	exit 1
endif
if !(-e ../../bootstrap/bootstrap) then
	echo "you must build the bootstrap under ./src/bootstrap"
	exit 1
endif

echo ""
echo "ls -l ../../bootstrap/bootstrap"
ls -l ../../bootstrap/bootstrap
echo "ls -l mach_kernel"
ls -l mach_kernel
echo ""

if (-e mach.boot) rm -f mach.boot
echo "../../mach_services/cmds/makeboot/makeboot -o mach.boot mach_kernel ../../bootstrap/bootstrap"

../../mach_services/cmds/makeboot/makeboot -o mach.boot mach_kernel ../../bootstrap/bootstrap

if !(-e mach.boot) then
	echo "can't stat mach.boot - check for errors"
	exit 1
else 
	echo ""
	echo "ls -l mach.boot"
	ls -l mach.boot
endif

exit 0

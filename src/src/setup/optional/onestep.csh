#! /bin/csh -f

set pwd = `pwd`
set this_dir = `basename ${pwd}` 

if (${this_dir} != "src") then
	echo " Must be cd'd to ./src"
	exit 1
endif

rm -f *.log

# do export pass

echo "building ./src MAKEFILE_PASS=FIRST"
sh -xf setup/AT386/build.sh AT386 osf1 . MAKEFILE_PASS=FIRST >& first.log

# build libmach needed from ./mach_services

echo "building mach_services/lib/libmach"
sh -xf setup/AT386/build.sh AT386 osf1 mach_services/lib/libmach build_all >& libmach.log

echo "building mach_services/lib/libsa_mach" 
sh -xf setup/AT386/build.sh AT386 osf1 mach_services/lib/libsa_mach build_all >& libsa_mach.log

echo "building mach_services/lib/libcthreads"
sh -xf setup/AT386/build.sh AT386 osf1 mach_services/lib/libcthreads build_all >& libcthreads.log

# ./bootstrap

echo "building bootstrap"
sh -xf setup/AT386/build.sh AT386 osf1 bootstrap build_all >& bootstrap.log

# default_pager

echo "building default_pager"
sh -xf setup/AT386/build.sh AT386 osf1 default_pager build_all >& pager.log

# makeboot command

echo "building mach_services/cmds/makeboot"
sh -xf setup/AT386/build.sh AT386 osf1 mach_services/cmds/makeboot build_all >& mboot.log

# mach_kernel

echo "building mach_kernel"
sh -xf setup/AT386/build.sh AT386 osf1 mach_kernel build_all  >& uk.log

echo "grep for errors"
grep Error *.log

exit 0

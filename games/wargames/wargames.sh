#!/bin/sh -
#
# Copyright (c) 1985, 1993
#	The Regents of the University of California.  All rights reserved.
#
# %sccs.include.redist.sh%
#
#	@(#)wargames.sh	8.1 (Berkeley) 05/31/93
#
echo -n "Would you like to play a game? "
read x

if [ -f /usr/games/$x ] ; then
	tput cl
	exec /usr/games/$x
else
	echo "Funny, the only way to win is not to play at all."
fi
exit 0

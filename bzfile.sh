#!/bin/sh

set -e

case "$5" in
	"assay")
		[ -f "$1/$4.bz2" ] || exit 3
	;;

	"retrieve")
		bzip2 -d -c <"$1/$4.bz2"
	;;

	"deposit")
		bzip2 -z -c -9 >"$1/$4.bz2"
	;;

	"efface")
		rm -- "$1/$4.bz2"
	;;

	*)
		echo "Invalid storage operation “$5”!" >&2
		exit 2
	;;
esac

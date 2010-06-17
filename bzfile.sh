#!/bin/sh

set -e

case "$5" in
	"retrieve")
		bzip2 -d -c <"$1/$4"
	;;

	"deposit")
		bzip2 -z -c -9 >"$1/$4"
	;;

	"efface")
		rm -- "$1/$4"
	;;

	*)
		echo "Invalid storage operation “$5”!" >&2
		exit 1
	;;
esac

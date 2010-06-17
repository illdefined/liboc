#!/bin/sh

set -e

case "$5" in
	"retrieve")
		xz -d -c <"$1/$4"
	;;

	"deposit")
		xz -z -c -7 >"$1/$4"
	;;

	"efface")
		rm -- "$1/$4"
	;;

	*)
		echo "Invalid storage operation “$5”!" >&2
		exit 1
	;;
esac

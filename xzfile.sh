#!/bin/sh

set -e

case "$5" in
	"assay")
		[ -f "$1/$4.xz" ] || exit 3
	;;

	"retrieve")
		xz -d -c <"$1/$4.xz"
	;;

	"deposit")
		xz -z -c -7 >"$1/$4.xz"
	;;

	"efface")
		rm -- "$1/$4.xz"
	;;

	*)
		echo "Invalid storage operation “$5”!" >&2
		exit 1
	;;
esac

#!/bin/sh

set -e

case "$5" in
	"assay")
		[ -f "$1/$4" ] || exit 3
	;;

	"retrieve")
		cat "$1/$4"
	;;

	"deposit")
		>"$1/$4"
	;;

	"efface")
		rm -- "$1/$4"
	;;

	*)
		echo "Invalid storage operation “$5”!" >&2
		exit 2
	;;
esac

#!/bin/sh

set -e

case "$5" in
	"retrieve")
		gzip -d -c <"$1/$4.gz"
	;;

	"deposit")
		gzip -z -c -9 >"$1/$4.gz"
	;;

	"efface")
		rm -- "$1/$4.gz"
	;;

	*)
		echo "Invalid storage operation “$5”!" >&2
		exit 1
	;;
esac

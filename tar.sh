#!/bin/sh

set -e

archive="$1/opencorpus.tar"

case "$5" in
	"retrieve")
		tar -x -f "$archive" -C "$3" "$4"
		<"$3/$4"
		rm -- "$3/$4"
	;;

	"deposit")
		>"$3/$4"
		tar -r -f "$archive" -C "$3" "$4"
		rm -- "$3/$4"
	;;

	"efface")
		echo "The tar storage module does not support effacement!" >&2
		exit 1
	;;

	*)
		echo "Invalid storage operation “$5”!" >&2
		exit 1
	;;
esac

#!/bin/sh

set -e

archive="$1/corpus.tar"

case "$5" in
	"assay")
		tar -t -f "$archive" "$4" >/dev/null || exit 3
	;;

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

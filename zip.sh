#!/bin/sh

set -e

archive="$1/opencorpus.zip"

case "$5" in
	"retrieve")
		unzip -p "$archive" "$4"
	;;

	"deposit")
		>"$3/$4"
		zip -b "$3" -j -m "$archive" "$3/$4"
	;;

	"efface")
		zip -b "$3" -d "$archive" "$4"
	;;

	*)
		echo "Invalid storage operation “$5”!" >&2
		exit 1
	;;
esac

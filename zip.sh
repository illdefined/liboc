#!/bin/sh

set -e

archive="$1/corpus.zip"

case "$5" in
	"assay")
		unzip -l "$archive" "$4" >/dev/null || exit 3
	;;

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
		exit 2
	;;
esac

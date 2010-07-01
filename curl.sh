#!/bin/sh

set -e

. /etc/opencorpus/storage/curl

if [ -z "${BASE_URI}" ]
then
	echo "No base URI specified!" >&2
	exit 1
fi

if [ -n "${TIMEOUT}" ]
then
	CURL_OPTS="${CURL_OPTS} --connect-timeout '${TIMEOUT}'"
fi

if [ -n "${CERT}" ]
then
	CURL_OPTS="${CURL_OPTS} --cert '${CERT}'"
fi

if [ -n "${KEY}" ]
then
	CURL_OPTS="${CURL_OPTS} --key '${KEY}'"
fi

if [ -n "${PASS}" ]
then
	CURL_OPTS="${CURL_OPTS} --pass '${PASS}'"
fi

if [ -n "${AUTH}" ]
then
	CURL_OPTS="${CURL_OPTS} --user '${AUTH}'"
fi

case "$5" in
	"assay")
		case "${BASE_URI%%://*}" in
			http|https)
				curl ${CURL_OPTS} -I "${BASE_URI}$4" | head -n 1 \
				| grep -E -q '^HTTP/1\.1 2[0-9][0-9] .+$' \
				|| exit 3
			;;

			ftp|ftps|sftp)
				curl ${CURL_OPTS} -I "${BASE_URI}$4" || exit 3
			;;

			*)
				echo "The selected protocol does not support assaying!" >&2
				exit 1
			;;
		esac
	;;

	"retrieve")
		curl ${CURL_OPTS} -G "${BASE_URI}$4"
	;;

	"deposit")
		curl ${CURL_OPTS} -T - "${BASE_URI}$4"
	;;

	"efface")
		case "${BASE_URI%%://*}" in
			http|https)
				curl ${CURL_OPTS} -X DELETE "${BASE_URI}$4"
			;;

			ftp|ftps|sftp)
				curl ${CURL_OPTS} -Q "rm $4" "${BASE_URI}"
			;;

			*)
				echo "The selected protocol does not support effacement!" >&2
				exit 1
			;;
		esac
	;;

	*)
		echo "Invalid storage operation “$5”!" >&2
		exit 1
	;;
esac

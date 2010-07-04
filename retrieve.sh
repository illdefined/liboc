#!/bin/sh

set -e

if [ $# -ne 2 ]
then
	echo "Invalid number of arguments" >&2
	exit 1
fi

# Find storage module
if [ -n "$HOME" -a -x "$HOME/.opencorpus/storage/$1" ]
then
	module="$HOME/.opencorpus/storage/$1"
elif [ -x "/usr/libexec/opencorpus/storage/$1" ]
then
	module="/usr/libexec/opencorpus/storage/$1"
else
	echo "Cannot find storage module" >&2
	exit 1
fi

# Create cache directory
if [ -w "/var/cache/opencorpus/storage" ]
then
	cache="/var/cache/opencorpus/storage/$1"
elif [ -n "$HOME" ]
then
	cache="$HOME/.opencorpus/cache/storage/$1"
else
	echo "Unable to create cache directory" >&2
	exit 1
fi

mkdir -p "$cache"

# Create temp directory
if [ -w "/var/tmp/opencorpus/storage" ]
then
	temp=`mktemp -d "/var/tmp/opencorpus/storage/$2-XXXXXXXX"`
else
	temp=`mktemp -d`
fi

mkdir -p "$temp"

# Clean temp directory upon exit
trap 'rm -f -r -- "$temp"' EXIT

# Launch storage module
if [ -z "$NO_SANDBOX" ]
then
	export SYDBOX_WRITE="/dev/fd:/dev/full:/dev/null:/dev/stderr:/dev/stdout:/dev/shm:/dev/tty:/dev/zero:/proc/self/attr:/proc/self/fd:/proc/self/task:/tmp:$cache:$temp"
#	export SYDBOX_EXEC="${PATH}:$transform:$runtime"
#	export SYDBOX_NET_WHITELIST_BIND="LOCAL6@0-65535;LOCAL@0-65535"
#	export SYDBOX_NET_WHITELIST_CONNECT="$SYDBOX_NET_WHITELIST_BIND"

	# Try to retrieve from local storage
	if sydbox -C -L -B "$module" "$HOME/.opencorpus/corpus/$1" "$cache" "$temp" "$2" "assay"
	then
		sydbox -C -L -B "$module" "$HOME/.opencorpus/corpus/$1" "$cache" "$temp" "$2" "retrieve"
	else
		sydbox -C -L -B "$module" "/var/db/opencorpus/$1" "$cache" "$temp" "$2" "retrieve"
	fi
else
	# Try local storage
	if "$module" "$HOME/.opencorpus/corpus/$1" "$cache" "$temp" "$2" "assay"
	then
		"$module" "$HOME/.opencorpus/corpus/$1" "$cache" "$temp" "$2" "retrieve"
	else
		"$module" "/var/db/opencorpus/$1" "$cache" "$temp" "$2" "retrieve"
	fi
fi

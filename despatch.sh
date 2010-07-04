#!/bin/sh

set -e

if [ $# -ne 2 ]
then
	echo "Invalid number of arguments" >&2
	exit 1
fi

# Find transformation
if [ -n "$HOME" -a -d "$HOME/.opencorpus/transform/$1" ]
then
	transform="$HOME/.opencorpus/transform/$1"
elif [ -d "/usr/share/opencorpus/$1" ]
then
	transform="/usr/share/opencorpus/$1"
else
	echo "Cannot find transformation “$1”" >&2
	exit 1
fi

# Find runtime environment
if [ -f "$transform/runtime" ]
then
	runtime=`< "$transform/runtime"`
	if [ -n "$HOME" -a -x "$HOME/.opencorpus/runtime/$runtime" ]
	then
		runtime="$HOME/.opencorpus/runtime/$runtime"
	elif [ -x "/usr/libexec/opencorpus/runtime/$runtime" ]
	then
		runtime="/usr/libexec/opencorpus/runtime/$runtime"
	else
		echo "Cannot find transformation runtime environment “$runtime”" >&2
		exit 1
	fi
elif [ -x "$transform/transform" ]
	then
	runtime="$transform/transform"
else
	echo "Cannot determine runtime environment" >&2
	exit 1
fi

# Create cache directory
if [ -w "/var/cache/opencorpus/runtime" ]
then
	cache="/var/cache/opencorpus/runtime/`basename $runtime`"
elif [ -n "$HOME" -a -d "$HOME/.opencorpus" ]
then
	cache="$HOME/.opencorpus/cache/runtime/`basename $runtime`"
else
	echo "Unable to create cache directory" >&2
	exit 1
fi

mkdir -p "$cache"

# Create temp directory
if [ -w "/var/tmp/opencorpus/runtime" ]
then
	temp=`mktemp -d "/var/tmp/opencorpus/runtime/$1-XXXXXXXX"`
else
	temp=`mktemp -d`
fi

mkdir -p "$temp"

# Despatch transformation
if [ -z "$NO_SANDBOX" ]
then
	export SYDBOX_WRITE="/dev/fd:/dev/full:/dev/null:/dev/stderr:/dev/stdout:/dev/shm:/dev/tty:/dev/zero:/proc/self/attr:/proc/self/fd:/proc/self/task:/tmp:$cache:$temp"
#	export SYDBOX_EXEC="${PATH}:$transform:$runtime"
#	export SYDBOX_NET_WHITELIST_BIND="LOCAL6@0-65535;LOCAL@0-65535"
#	export SYDBOX_NET_WHITELIST_CONNECT="$SYDBOX_NET_WHITELIST_BIND"

	sydbox -C -L -B "$runtime" "$transform" "$cache" "$temp" "$2"
else
	"$runtime" "$transform" "$cache" "$temp" "$2"
fi

# Clean temp directory
rm -f -r -- "$temp"

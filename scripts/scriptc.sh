#!/bin/sh -e
# The scripty C preprocessor
# Searches for #pragma script <script> and generates code at that point using the script specified.

match_pragma() {
	# Detect if a string starts with a script pragma

	# Check that the line starts with '#'
	[ "${line#'#'}" = "$line" ] && return 1

	cmd="$(echo "${line#'#'}" | {
		# Pull out the first two fields and the rest of it
		IFS=' ' read -r pragma script cmd
		# Set the variable to "the rest of it"
		echo "$cmd"
		# Check the first two fields are correct
		[ "$pragma" = pragma ] && [ "$script" = script ]
	})" || return 1
}

if [ "$#" -lt 2 ]; then
	echo "Usage: $0 INPUT OUTPUT" >&2
	exit 1
fi

input="$1"
output="$2"

# We use tr to strip carriage returns in case there's Windowsy files in the repo
tr -d '\r' <"$input" | while read -r line; do
	if match_pragma "$line"; then
		eval "$cmd" || {
			echo "Command '$cmd' failed. Deleting output file." >&2
			rm -- "$output"
			exit 1
		}
	else
		echo "$line"
	fi
done >"$output"

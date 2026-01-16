#!/bin/bash

files=(./*.3)
total=${#files[@]}
cpus=$(nproc)

# Progress file
progress_file=$(mktemp)
trap 'rm -f "$progress_file"' EXIT

process_file() {
    local file="$1"
    local output="${file}.pdf"
    local md5_file="${file}.md5"

    current_md5=$(md5sum "$file" | awk '{print $1}')
    prev_md5=""
    [[ -f "$md5_file" ]] && prev_md5=$(<"$md5_file")

    if [[ "$current_md5" != "$prev_md5" ]]; then
        man -l -Tpdf "$file" > "$output"
        echo "$current_md5" > "$md5_file"
        msg="Built $(basename "$output")"
    else
        msg="Skipped $(basename "$output")"
    fi

    # record completion
    echo "$msg" >> "$progress_file"
}

export -f process_file
export progress_file

# Zenity progress loop
(
    while :; do
        completed=$(wc -l < "$progress_file")
        percent=$((completed * 100 / total))

        if (( completed >= total )); then
            echo "100"
            echo "# Done"
            break
        fi

        last=$(tail -n1 "$progress_file")
        [[ -z "$last" ]] && last="Starting…"
        echo "$percent"
        echo "# $last"

        sleep 0.2
    done
) | zenity --progress \
          --title="Building PDFs" \
          --text="Starting…" \
          --percentage=0 \
          --auto-close \
          --width=450 &

zenity_pid=$!

# Start jobs with a simple job slot limiter
active=0
for file in "${files[@]}"; do
    process_file "$file" &
    active=$((active+1))
    if (( active >= cpus )); then
        wait -n
        active=$((active-1))
    fi
done

# wait for all jobs
wait

# wait for Zenity to finish
wait "$zenity_pid"

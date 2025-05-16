#!/bin/bash

output_file="final_output.txt"

# Clear the output file before starting
> "$output_file"

for thread_num in 1 2 4 8 16 32; do
    for x in {1..5}; do
        command="./atsp_pth $thread_num 256"

        echo "Executing: $command" >> "$output_file"
        $command >> "$output_file" 2>&1
        echo "-------------------------------" >> "$output_file"
        echo "" >> "$output_file"
        echo "" >> "$output_file"
    done
done

echo "All commands executed and output saved to $output_file"

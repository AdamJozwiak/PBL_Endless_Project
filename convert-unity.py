import sys

if len(sys.argv) <= 1:
    exit()

input_filename = sys.argv[1]
output_filename = sys.argv[2] if len(sys.argv) >= 3 else input_filename

input_lines = []
with open(input_filename, "r") as input_file:
    input_lines = [line for line in input_file]

with open(output_filename, "w") as output_file:
    for i in range(len(input_lines)):
        output_file.write(input_lines[i])

        if (
            i < 1
            or i >= len(input_lines) - 1
            or input_lines[i - 1].find("---") == -1
            or input_lines[i + 1].find("  id: ") != -1
        ):
            continue

        output_file.write(
            "  id: " + input_lines[i - 1][input_lines[i - 1].find("&") + 1 :]
        )

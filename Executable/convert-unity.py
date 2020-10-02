# Imports
import sys
import pathlib
import ruamel.yaml
import json

# Read program arguments
arguments = None
if len(sys.argv) == 1:
    arguments = ["."]
else:
    arguments = sys.argv[1:]

# Transform arguments into paths
input_paths = [pathlib.Path(argument) for argument in arguments]

# Make a list of all files to convert
filenames = []
for input_path in input_paths:
    if input_path.is_dir():
        for extension in ["*.unity", "*.prefab", "*.mat"]:
            filenames += [str(path) for path in input_path.rglob(extension)]
    else:
        filenames.append(str(input_path))

# Convert files
for filename in filenames:
    # Read file's contents
    input_lines = []
    with open(filename, "r") as input_file:
        input_lines = [line for line in input_file]

    # Overwrite contents in-place with converted version
    with open(filename, "w") as output_file:
        for i in range(len(input_lines)):
            # Print conversion status
            print(
                "|",
                round((i * 10) / len(input_lines)) * "-",
                (10 - round((i * 10) / len(input_lines))) * " ",
                "| ",
                filename,
                sep="",
                end="\r",
            )

            # Change YAML version
            if input_lines[i].startswith("%YAML"):
                input_lines[i] = "%YAML 1.2\n"

            # Omit the tag line
            if input_lines[i].startswith("%TAG"):
                continue

            # Remove tag and "stripped" keyword
            if input_lines[i].startswith("--- !u!"):
                input_lines[i] = (
                    input_lines[i].split(" ")[0] + " " + input_lines[i].split(" ")[2]
                ).strip() + "\n"

            # Write corrected line
            output_file.write(input_lines[i])

            # Skip this line if conversion is not applicable
            if (
                i < 1
                or i >= len(input_lines) - 1
                or input_lines[i - 1].find("---") == -1
                or input_lines[i + 1].find("  id: ") != -1
            ):
                continue

            # Add a new line with identifier
            output_file.write(
                "  id: " + input_lines[i - 1][input_lines[i - 1].find("&") + 1 :]
            )

    # Convert YAML to JSON
    yaml = ruamel.yaml.YAML(typ="safe")
    with open(filename, "r") as input_file:
        data = list(yaml.load_all(input_file))
    with open(filename, "w") as output_file:
        json.dump(data, output_file, separators=(",", ":"))

    # Print new line
    print()

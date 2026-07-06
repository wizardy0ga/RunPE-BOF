#!/bin/bash

# Check if directory argument was provided
if [ $# -eq 0 ]; then
    echo "Error: No directory argument provided. Provide the full path to the root of your Havoc installation."
    echo "Usage: $0 <havoc_root_dir>"
    exit 1
fi

# Get the havoc root directory from argument
havoc_root_dir="$1"

# Validate that the directory exists
if [ ! -d "$havoc_root_dir" ]; then
    echo "Error: Directory '$havoc_root_dir' does not exist."
    exit 1
fi

# Define paths
extensions_dir="$havoc_root_dir/data/extensions"
runpe_dir="$extensions_dir/RunPE"
source_c_file="../Source/BOF/bof.c"
script_py_file="../Source/Scripts/RunPE.py" # Note: The file name was changed from RunPE.py to runpe.py for consistency.
dist_o_file="../Dist/RunPE.x64.o"

# Create RunPE directory
echo "Creating directory: $runpe_dir"
mkdir -p "$runpe_dir"

# Check if x86_64-w64-mingw32-gcc is installed
if command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo "x86_64-w64-mingw32-gcc found. Compiling bof.c..."
    
    # Compile the source file
    if x86_64-w64-mingw32-gcc -o "$runpe_dir/RunPE.x64.o" -c "$source_c_file"; then
        echo "Successfully compiled bof.c to RunPE.x64.o"
    else
        echo "Error: Failed to compile bof.c"
        exit 1
    fi
else
    echo "x86_64-w64-mingw32-gcc not found. Copying pre-compiled object file..."
    
    # Check if the dist file exists
    if [ -f "$dist_o_file" ]; then
        cp "$dist_o_file" "$runpe_dir/"
        echo "Copied $dist_o_file to $runpe_dir/"
    else
        echo "Error: Pre-compiled file $dist_o_file not found"
        exit 1
    fi
fi

# Copy the Python script
echo "Copying runpe.py script..."
cp "$script_py_file" "$runpe_dir/"

# Update object_file_path variable in runpe.py
object_file_path="$havoc_root_dir/data/extensions/RunPE/RunPE.x64.o"
sed -i "s#object_file_path    = \"\"#object_file_path    = \"$object_file_path\"#" "$runpe_dir/RunPE.py"

echo ""
echo "========================================"
echo "Setup completed successfully!"
echo ""
echo "To load the script in Havoc console:"
echo "1. Launch Havoc"
echo "2. Navigate to Script Manager"
echo "3. Load runpe.py from $runpe_dir"
echo "4. Execute 'help runpe' to verify installation in console"
echo "========================================"
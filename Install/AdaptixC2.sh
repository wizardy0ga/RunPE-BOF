# Check if directory argument was provided
if [ $# -eq 0 ]; then
    echo "Error: No directory argument provided. Provide the full path to the root of the RunPE-BOF repo."
    echo "Usage: $0 <RunPE-BOF repo dir.>"
    exit 1
fi

# Get the havoc root directory from argument
repo_path="$1"

# Validate that the directory exists
if [ ! -d "$repo_path" ]; then
    echo "Error: Directory '$1' does not exist."
    exit 1
fi

# Check if compiler is installed & compile
if command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo "x86_64-w64-mingw32-gcc found. Compiling bof.c..."
    
    if x86_64-w64-mingw32-gcc -o "../Dist/RunPE.x64.o" -c "../Source/BOF/bof.c"; then
        echo "Successfully compiled bof.c to RunPE.x64.o"
    else
        echo "Error: Failed to compile bof.c"
        exit 1
    fi
else
    echo "x86_64-w64-mingw32-gcc not found. Using pre-compiled object file..."
fi

# Update bof path
sed -i "s#bof_path = \"\";#bof_path = \"$1/Dist/RunPE.x64.o\";#" ../Source/Scripts/RunPE.axs

echo ""
echo "========================================"
echo "Setup completed successfully!"
echo ""
echo "To load the script in Adaptix C2:"
echo "1. Open Adaptix Client"
echo "2. Navigate to Extensions > Script manager"
echo "3. Load the RunPE extension from: Source/Scripts/RunPE.axs"
echo "========================================"
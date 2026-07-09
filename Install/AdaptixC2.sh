# Check if x86_64-w64-mingw64-gcc is installed
if command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo "x86_64-w64-mingw32-gcc found. Compiling bof.c..."
    
    # Compile the source file
    if x86_64-w64-mingw32-gcc -o "../Dist/RunPE.x64.o" -c "../Source/BOF/bof.c"; then
        echo "Successfully compiled bof.c to RunPE.x64.o"
    else
        echo "Error: Failed to compile bof.c"
        exit 1
    fi
else
    echo "x86_64-w64-mingw32-gcc not found. Using pre-compiled object file..."
fi

echo ""
echo "========================================"
echo "Setup completed successfully!"
echo ""
echo "To load the script in Adaptix C2:"
echo "1. Open Adaptix Client"
echo "2. Navigate to Extensions > Script manager"
echo "3. Load the RunPE extension from: Source/Scripts/RunPE.axs"
echo "========================================"
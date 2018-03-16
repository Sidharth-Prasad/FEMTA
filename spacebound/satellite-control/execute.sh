
rm ./shell-processes/*.txt

# Run shell processes in the background
cd ./shell-processes/
./*.sh &
cd ..

# Send start signal to shell processes
touch ./shell-processes/start.txt

# Start the C program
sudo ./unified-controller

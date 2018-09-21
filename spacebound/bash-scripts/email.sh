cd /home/noah/FEMTA/spacebound/bash-scripts

echo "My IP address is: $(hostname -I)" > current_ip_state.txt

cmp --silent current_ip_state.txt previous_ip_state.txt || cat current_ip_state.txt | mail -s "Raspberry Pi" nfranks@purdue.edu

cmp --silent current_ip_state.txt previous_ip_state.txt || cat current_ip_state.txt | mail -s "Raspberry Pi" Kfowee@purdue.edu

cmp --silent current_ip_state.txt previous_ip_state.txt || cat current_ip_state.txt | mail -s "Raspberry Pi" mason67@purdue.edu

cmp --silent current_ip_state.txt previous_ip_state.txt || cat current_ip_state.txt | mail -s "Raspberry Pi" jkezon@purdue.edu

cmp --silent current_ip_state.txt previous_ip_state.txt || cat current_ip_state.txt | mail -s "Raspberry Pi" agovindh@purdue.edu

cp current_ip_state.txt previous_ip_state.txt



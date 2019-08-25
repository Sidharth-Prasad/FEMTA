cd /home/noah/FEMTA/spacebound/helper-scripts/

echo "Hi, I'm a Pi Zero and hope to be on a Blue Origin rocket.\nMy favorite color is Blue. My favorite position on the cartesian plane is the origin. I hope saying that makes me more likely to be the unit that ends up on the actual craft.\nMy IP address is: $(hostname -I)" > current_ip_state.txt

cmp --silent ./current_ip_state.txt ./previous_ip_state.txt || cat ./current_ip_state.txt | mail -s "Raspberry Pi Zero" nfranks@purdue.edu

cmp --silent ./current_ip_state.txt ./previous_ip_state.txt || cat ./current_ip_state.txt | mail -s "Raspberry Pi Zero" Kfowee@purdue.edu

cmp --silent ./current_ip_state.txt ./previous_ip_state.txt || cat ./current_ip_state.txt | mail -s "Raspberry Pi Zero" jkezon@purdue.edu

cmp --silent ./current_ip_state.txt ./previous_ip_state.txt || cat ./current_ip_state.txt | mail -s "Raspberry Pi Zero" agovindh@purdue.edu

cp ./current_ip_state.txt ./previous_ip_state.txt



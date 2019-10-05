#!/usr/bin/env bash
cd /home/noah/FEMTA/spacebound/blue-origin/
tmux new-session -d -s origin 'sudo chrt 32 ./origin.x file=experiments/freeze.e | tee auto.log'

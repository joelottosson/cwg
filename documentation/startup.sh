#!/bin/bash

# cleanup old instances
pkill tank_player_cpp
pkill tank_sample
pkill java
pkill mono
pkill tank_game_gui
pkill tank_engine
pkill dope_main
pkill dose_main

mate-terminal --show-menubar --geometry=120x100 \
  --window --title "dose" -e "bash -c 'dose_main'" \
  --tab-with-profile=Default --title "dope" -e "bash -c 'dope_main'" \
  --tab-with-profile=Default --title "tank_engine" -e "bash -c 'sleep 3 && tank_engine'" \
  --tab-with-profile=Default --title "tank_match_server" -e "bash -c 'sleep 3 && tank_match_server'" \
  --tab-with-profile=Default --title "tank_game_gui" \
  --tab-with-profile=Default --title "Player1" \
  --tab-with-profile=Default --title "Player2" \
  --tab-with-profile=Default --title "Builder" 

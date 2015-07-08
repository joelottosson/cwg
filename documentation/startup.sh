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

gnome-terminal --show-menubar --geometry=120x100 \
  --tab --title "dose" -e "bash -c 'dose_main'" \
  --tab-with-profile=Default --title "dope" -e "bash -c 'dope_main'" \
  --tab-with-profile=Default --title "tank_engine" -e "bash -c 'tank_engine'" \
  --tab-with-profile=Default --title "tank_game_gui" \
  --tab-with-profile=Default --title "Player1" \
  --tab-with-profile=Default --title "Player2" \
  --tab-with-profile=Default --title "Builder" 

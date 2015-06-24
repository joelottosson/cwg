# im not good at writing good Makefiles. Maybe that will change someday :)

engine: .FORCE
	cd src/tank_engine/src && cmake . && make install
	@cd ../../..
	
gui: .FORCE
	cd src/tank_game_gui/src && cmake . && make install
	@cd ../../..
	
server: .FORCE
	cd src/tank_match_server/src && cmake . && make install
	@cd ../../..
	
random_player: .FORCE
	@cd src/tank_random_player/src && cmake . && make install
	@cd ../../..

cool_players: .FORCE
	cd src/players/tank_random_player/src && cmake . && make install
	@cd ../../..
	cd src/players/tank_player_dumb/src && cmake . && make install
	@cd ../../..
	cd src/players/tank_joy_player/src && cmake . && make install
	@cd ../../..
	cd src/players/tank_player_grandmother/src && cmake . && make install
	@cd ../../..
	cd src/players/tank_player_templates/tank_player_cpp/src && cmake . && make install
	@cd ../../..

backend: remove_cache engine gui server
	
dob: .FORCE
	dobmake.py --no-ada --no-java --no-dotnet --batch
	

#How do i stop !?
start_backend: .FORCE
	$(TERM) -e sh init.sh &
	
start_complete: .FORCE
	$(TERM) -e sh init-players.sh &

remove_cache: .FORCE
	-find . -name CMakeCache.txt | xargs rm	

copy_and_rebuid_dob: .FORCE
	 cp -uv tank_dou/*.dou $(SAFIR_SDK)/dots/dots_generated/tank/
	 dobmake.py --no-java --no-ada --no-dotnet --batch
	 

#Fix this later
.FORCE: 
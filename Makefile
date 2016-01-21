# im not good at writing good Makefiles. Maybe that will change someday :)

FLAGS= 



engine: .FORCE
	cd src/tank_engine/src && cmake -DMYFLAGS='$(FLAGS)' . && make install
	@cd ../../..
	
gui: .FORCE
	cd src/tank_game_gui/src && cmake -DMYFLAGS='$(FLAGS)' . && make  install
	@cd ../../..
	
server: .FORCE
	cd src/tank_match_server/src && cmake -DMYFLAGS='$(FLAGS)' . && make install
	@cd ../../..
	
	
clean_engine: .FORCE
	cd src/tank_engine/src && cmake . && make clean
	@cd ../../..
	
clean_gui: .FORCE
	cd src/tank_game_gui/src && cmake . && make clean
	@cd ../../..
	
clean_server: .FORCE
	cd src/tank_match_server/src && cmake . && make clean
	@cd ../../..
	
random_player: .FORCE
	@cd src/tank_random_player/src && cmake . && make install
	@cd ../../..

cool_players: .FORCE
	cd src/players/tank_random_player/src && cmake . && make install
	@cd ../../..
	cd src/players/tank_player_dumb/src && cmake . && make install
	@cd ../../..
	cd src/players/tank_player_manual/src && cmake . && make install
	@cd ../../..
	cd src/players/tank_player_grandmother/src && cmake . && make install
	@cd ../../..
	cd src/players/tank_player_templates/tank_player_cpp/src && cmake . && make install
	@cd ../../..
	cd src/players/tank_player_templates/tank_player_java/src && cmake . && make install
	@cd ../../..
		cd src/players/tank_player_templates/tank_player_cs/src && cmake . && make install
	@cd ../../..
	cd src/players/tank_player_powerup/src && cmake . && make install
	@cd ../../..

	
clean_cool_players: .FORCE
	cd src/players/tank_random_player/src && cmake . && make clean
	@cd ../../..
	cd src/players/tank_player_dumb/src && cmake . && make clean
	@cd ../../..
	cd src/players/tank_player_manual/src && cmake . && make clean
	@cd ../../..
	cd src/players/tank_player_grandmother/src && cmake . && make clean
	@cd ../../..
	cd src/players/tank_player_templates/tank_player_cpp/src && cmake . && make clean
	@cd ../../..

backend: engine gui server

clean_backend: clean_engine clean_gui clean_server

clean_all: clean_engine clean_gui clean_server clean_cool_players

all: .FORCE copy_and_rebuild_dob engine gui server cool_players 


remove_cache: .FORCE
	-find . -name CMakeCache.txt | xargs rm	

copy_and_rebuild_dob_cpp: .FORCE
	 mkdir -p $(SAFIR_SDK)/dots/dots_generated/tank_game/
	 cp -uv tank_dou/*.dou $(SAFIR_SDK)/dots/dots_generated/tank_game/
	 dobmake.py --no-java --no-ada --no-dotnet --batch
	 
copy_and_rebuild_dob: .FORCE
	 mkdir -p $(SAFIR_SDK)/dots/dots_generated/tank_game/
	 cp -uv tank_dou/*.dou $(SAFIR_SDK)/dots/dots_generated/tank_game/
	 dobmake.py --no-ada  --batch

targets:
	@echo "all                        - builds everything including the dobs and the default players"
	@echo "backend                    - builds only the engine, match server, the gui and the config system"
	@echo "cool_players               - builds the default players"
	@echo "clean_all                  - cleans everything except the backend"
	@echo "gui|engine|server          - builds a specific module"
	@echo "clean_*                    - cleans a specific module where * is the name of the module"
	@echo "copy_and_rebuild_dob       - copies dou files from the tank_dou to the runtime path and rebuilds them"



	 
.FORCE: 

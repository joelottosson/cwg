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
	
config: .FORCE
	cd src/ && cmake . && make install
	@cd ../
	
	
	
clean_config: .FORCE
	cd src/ && cmake . && make clean
	@cd ../

	
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

backend: config engine gui server

clean_backend: clean_engine clean_gui clean_server clean_config

clean_all: clean_engine clean_gui clean_server clean_cool_players clean_config

all: .FORCE copy_and_rebuild_dob config engine gui server cool_players 


remove_cache: .FORCE
	-find . -name CMakeCache.txt | xargs rm	

copy_and_rebuid_dob_cpp: .FORCE
	 cp -uv tank_dou/*.dou $(SAFIR_SDK)/dots/dots_generated/tank/
	 dobmake.py --no-java --no-ada --no-dotnet --batch
	 
copy_and_rebuid_dob: .FORCE
	 cp -uv tank_dou/*.dou $(SAFIR_SDK)/dots/dots_generated/tank/
	 dobmake.py --no-ada  --batch

targets:
	@echo "all                        - builds everything including the dobs and the default players"
	@echo "backend                    - builds only the engine,match server ,the gui and the config system"
	@echo "cool_players               - builds the default players"
	@echo "clean_all                  - cleans everything except the dobs"
	@echo "gui|engine|server|config   - builds a specific module"
	@echo "clean_*                    - cleans a specific module where * is the name of the module"
	@echo "copy_and_rebuild_dob       - copies dou files from the tank_dou to the runtime path and rebuilds them"



	 
.FORCE: 

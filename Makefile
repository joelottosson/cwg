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
	cd src/tank_random_player/src && cmake . && make install
	@cd ../../..

backend: engine gui server

#How do i stop !?
start_backend: .FORCE
	$(TERM) -e sh init.sh &
	
start_complete: .FORCE
	$(TERM) -e sh init-players.sh &

	

#Fix this later
.FORCE: 
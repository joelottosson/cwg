# im not good at writing good Makefiles. Maybe that will change someday :)

engine: .FORCE
	cd tank_engine/src && cmake . && make install
	@cd ../..
	
gui: .FORCE
	cd tank_game_gui/src && cmake . && make install
	@cd ../..
	
server: .FORCE
	cd tank_match_server/src && cmake . && make install
	@cd ../..
	
random_player: .FORCE
	cd tank_random_player/src && cmake . && make install
	@cd ../..

backend: engine gui server

#How do i stop !?
start_backend: .FORCE
	$(TERM) -e sh init.sh &
	
start_complete: .FORCE
	$(TERM) -e sh init-players.sh &

	

#Fix this later
.FORCE: 
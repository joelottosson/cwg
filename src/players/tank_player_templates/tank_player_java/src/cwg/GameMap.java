/******************************************************************************
*
* Copyright Consoden AB, 2014
*
* Created by: Joel Ottosson / joot
*
******************************************************************************/
package cwg;

import java.util.Calendar;


//Contains helper methods for reading a GameState and moving around in the game.
class GameMap {
	
	private int tankId;
	private consoden.tankgame.GameState gameState;
	private Calendar startOfDay;
	
	private byte mine=(byte)'o';
	private byte wall=(byte)'x';
	private byte coin=(byte)'$';
	private byte poison=(byte)'p';
	private byte laser_ammo=(byte)'l';
	private byte smoke_grenade=(byte)'s';
	private byte redeemer_ammo=(byte)'r';
	
	//Constructor, creates a new GameMap from a GameState.
	public GameMap(int tankId, consoden.tankgame.GameState gameState) {		
		this.tankId=tankId;
		this.gameState=gameState;
		
		startOfDay=Calendar.getInstance();
		int year=startOfDay.get(Calendar.YEAR);
		int month=startOfDay.get(Calendar.MONTH);
		int date=startOfDay.get(Calendar.DATE);
		startOfDay.clear();
		startOfDay.set(year, month, date);
	}
	
	//X size of game field.
	public int getSizeX() {
		return gameState.width().getVal();
	}
	
	//Y size of game field.
	public int getSizeY() {
		return gameState.height().getVal();
	}
	
	//Own tank position.
	public Position getOwnPosition() {
		return new Position(gameState.tanks().get(tankId).getObj().posX().getVal(),
				gameState.tanks().get(tankId).getObj().posY().getVal());
	}
	
	//How many lasers do we have?
	public int getLaserCount() {
		return gameState.tanks().get(tankId).getObj().lasers().getVal();

	}

	//Enemy tank position.
	public Position getEnemyPosition() {
		int id=(tankId+1)%2;
		if(gameState.tanks().get(id).getObj().smokeLeft().getVal() >0){
			return new Position((int)(Math.random() * gameState.width().getVal() + 1),
					(int)(Math.random() * gameState.height().getVal() + 1));
		}else{
			return new Position(gameState.tanks().get(id).getObj().posX().getVal(),
				gameState.tanks().get(id).getObj().posY().getVal());
		}
	}
	
	//Functionality identical to GetMissile	
	public consoden.tankgame.Redeemer GetRedeemer(boolean enemy_tank){
		int id;
		if(enemy_tank){
			id = (tankId +1) % 2;
		}else{
			id = tankId;
		}
		 
		 
		for (int i=0; i<this.gameState.redeemers().size(); i++) {
			if (gameState.redeemers().get(i).isNull()){
				continue;
			}

			consoden.tankgame.Redeemer redeemer = gameState.redeemers().get(i).getObj();
			System.out.println(redeemer.tankId().getVal() + "and the target id is sorta like ish " + id);
			if (redeemer.tankId().getVal() == id) {
				return redeemer;
			}
		}
		return null;
	}
	 /*
	 * Returns an objet with information about the missile. If enemy_tank is set to false
	 * the missile of your own player will be returned otherwise its the enemys missile.
	 *
	 * If no missile exists the function will still return a missile object but it will be null.
	 * To get the other field you need to use getter methods wich are just the name of the field you want to get.
	 *
	 * Se the game documentation for details about the different fields.
	 *
	 * example:
	 *	consoden.tankgame.Missile missile = gm.GetMissile(false);
	 *   if(missile != null){
	 *	    System.out.println("enemy missile head position is " + missile.headPosX().getVal() + "," + missile.headPosY().getVal() );
	 *   }
	 *   
	 *
	 */
	public consoden.tankgame.Missile GetMissile(boolean enemy_tank){
		int id;
		if(enemy_tank){
			id = (tankId +1) % 2;
		}else{
			id = tankId;
		}
		 
		for (int i=0; i<this.gameState.missiles().size(); i++) {
			if (gameState.missiles().get(i).isNull()){
				continue;
			}
			consoden.tankgame.Missile missile = gameState.missiles().get(i).getObj();
			if (missile.tankId().getVal() == id) {
				return missile;
			}
		}
		return null;
	}
	
	//Enemy tank position.
	public boolean hasRedeemer() {
		return gameState.tanks().get(tankId).getObj().hasRedeemer().getVal();
	}
	
	//Do we have a smoke grenade ??
	public boolean hasSmoke(){
		return gameState.tanks().get(tankId).getObj().hasSmoke().getVal();
	}
	
	//Check if the Penguin is in this square.
	public boolean isPenguin(final Position pos) {
		return (pos.x == gameState.theDude().getObj().posX().getVal() &&  
				pos.y == gameState.theDude().getObj().posY().getVal());
	}

	// Check if the Penguin is still alive
	public boolean isPenguinAlive() {
		return !(gameState.theDude().getObj().dying().getVal());
	}
	
	//Check if square is a wall.
	public boolean isWall(final Position pos) {
		return gameState.board().getVal()[toIndex(pos)]==wall;
	}
	
	//Check if there is a mine in this square.
	public boolean isMine(final Position pos) {
		return gameState.board().getVal()[toIndex(pos)]==mine;
	}
	
	//Check if there is a mine in this square.
	public boolean isSmokeGrenade(final Position pos) {
		return gameState.board().getVal()[toIndex(pos)]==smoke_grenade;
	}
	
	//Check if there is a coin in this square.
	public boolean isCoin(final Position pos) {
		return gameState.board().getVal()[toIndex(pos)]==coin;
	}
	
	//Check if there is poison gas in this square.
	public boolean isPoisonGas(final Position pos) {
		return gameState.board().getVal()[toIndex(pos)]==poison;
	}
	
	//Check if there is poison gas in this square.
	public boolean isRedeemerAmmo(final Position pos) {
		return gameState.board().getVal()[toIndex(pos)]==redeemer_ammo;
	}
	

	
	
	//Returns the position of the penguin.
	public boolean isLaserAmmo(final Position pos) {
		return gameState.board().getVal()[toIndex(pos)]==laser_ammo;
	}

	//Is there a redeemer in this square
	public boolean isRedeemerInPosition (Position p) {
		for (int i=0; i<this.gameState.redeemers().size(); i++) {
			if (gameState.redeemers().get(i).isNull())
				continue;

			consoden.tankgame.Redeemer redeemer = gameState.redeemers().get(i).getObj();
			
			if (p.x == redeemer.posX().getVal() && p.y == redeemer.posY().getVal()) {
				return true;
			}
		}
		return false;
	}
	
	
	//Is there a missile in this square
	public boolean isMissileInPosition (Position p) {
		for (int i=0; i<this.gameState.missiles().size(); i++) {
			if (gameState.missiles().get(i).isNull())
				continue;

			consoden.tankgame.Missile missile = gameState.missiles().get(i).getObj();
			
			if ((p.x == missile.headPosX().getVal() && p.y == missile.headPosY().getVal()) ||
				(p.x == missile.tailPosX().getVal() && p.y == missile.tailPosY().getVal())) {
				return true;
			}
		}
		return false;
	}
	
	//Helper functions that moves position one step in the indicated direction.
	//Returns a new positon after the move operation.
	public Position move(final Position p, final consoden.tankgame.Direction d) {
		switch (d) {
        case LEFT:
        	return new Position((p.x-1+getSizeX())%getSizeX(), p.y);
        case RIGHT:
        	return new Position((p.x+1)%getSizeX(), p.y);
        case UP:
        	return new Position(p.x, (p.y-1+getSizeY())%getSizeY());
        case DOWN:
        	return new Position(p.x, (p.y+1)%getSizeY());
        case NEUTRAL:
        	return new Position(p.x, p.y);
        }
		return new Position(p.x, p.y);
	}
	
	//Milliseconds left until the joystick will be readout next time.
	public long timeToNextMove() {
		long time=Calendar.getInstance().getTimeInMillis()-startOfDay.getTimeInMillis();
		return gameState.nextMove().getVal().longValue()-time;
	}
	
	//Print game map
	public void printMap() {
		for (int y=0; y<getSizeY(); y++) {
			for (int x=0; x<getSizeX(); x++) {
				int i=toIndex(x, y);
				System.out.print((char)(gameState.board().getVal()[i]));
			}
			System.out.println();
		}
		System.out.println();
	}

	//Print game state
	public void printState() 
	{
	    System.out.println("Game board");
	    printMap();

	    System.out.println("Own position " + getOwnPosition());
	    System.out.println("Enemy position " + getEnemyPosition());

	    System.out.println("Active missiles");

		for (int i=0; i<this.gameState.missiles().size(); i++) {
			if (gameState.missiles().get(i).isNull())
				continue;

			consoden.tankgame.Missile missile = gameState.missiles().get(i).getObj();

			Position head = new Position(missile.headPosX().getVal(), missile.headPosY().getVal());
			Position tail = new Position(missile.tailPosX().getVal(), missile.tailPosY().getVal());
	        consoden.tankgame.Direction direction = missile.direction().getVal();

	        System.out.println("Missile position - head " + head);
	        System.out.println("Missile position - tail " + tail);
	        System.out.print("Missile direction ");

	        switch (direction) {
	        case LEFT:
	            System.out.println("Left");
	            break;
	        case RIGHT:
	            System.out.println("Right");
	            break;
	        case UP:
	            System.out.println("Up");
	            break;
	        case DOWN:
	            System.out.println("Down");
	            break;
	        case NEUTRAL:
	        	System.out.println("Neutral");
	            break;
	        }
	        System.out.println();
	    }

	    System.out.println();
	}

	private int toIndex(int x, int y) {
		return x+y*getSizeX();
	}
	
	private int toIndex(final Position pos) {
		return pos.x+pos.y*getSizeX();
	}
}

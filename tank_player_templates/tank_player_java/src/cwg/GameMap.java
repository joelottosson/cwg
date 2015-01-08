/******************************************************************************
*
* Copyright Consoden AB, 2014
*
* Created by: Joel Ottosson / joot
*
******************************************************************************/
package cwg;

import java.util.Calendar;

class GameMap {

	private long startTime;
	private int tankId;
	private consoden.tankgame.GameState gameState;
	private Calendar startOfDay;
	
	private byte mine=(byte)'o';
	private byte wall=(byte)'x';
	private byte coin=(byte)'$';
	private byte poison=(byte)'p';
	
	
	public GameMap(int tankId, consoden.tankgame.GameState gameState) {
		this.startTime=System.nanoTime();
		this.tankId=tankId;
		this.gameState=gameState;
		
		startOfDay=Calendar.getInstance();
		int year=startOfDay.get(Calendar.YEAR);
		int month=startOfDay.get(Calendar.MONTH);
		int date=startOfDay.get(Calendar.DATE);
		startOfDay.clear();
		startOfDay.set(year, month, date);
	}
	
	//X size of game field
	public int getSizeX() {
		return gameState.width().getVal();
	}
	
	//Y size of game field
	public int getSizeY() {
		return gameState.height().getVal();
	}
	
	//Own tank position
	public Position getOwnPosition() {
		return new Position(gameState.tanks().get(tankId).getObj().posX().getVal(),
				gameState.tanks().get(tankId).getObj().posY().getVal());
	}

	//Enemy tank position
	public Position getEnemyPosition() {
		int id=(tankId+1)%2;
		return new Position(gameState.tanks().get(id).getObj().posX().getVal(),
				gameState.tanks().get(id).getObj().posY().getVal());
	}
	
	//Check if square is a wall.
	public boolean isWall(final Position pos) {
		return gameState.board().getVal()[toIndex(pos)]==wall;
	}
	
	//Check if there is a mine in this square.
	public boolean isMine(final Position pos) {
		return gameState.board().getVal()[toIndex(pos)]==mine;
	}
	
	//Check if there is a coin in this square.
	public boolean isCoin(final Position pos) {
		return gameState.board().getVal()[toIndex(pos)]==coin;
	}
	
	//Check if there is poison gas in this square.
	public boolean isPoisonGas(final Position pos) {
		return gameState.board().getVal()[toIndex(pos)]==poison;
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
	
	//Milliseconds elapsed since creation of this instance
	public long getElapsedTime() {
		return (System.nanoTime()-startTime)/1000000;
	}
	
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

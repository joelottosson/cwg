/******************************************************************************
*
* Copyright Consoden AB, 2014
*
* Created by: Joel Ottosson / joot
*
******************************************************************************/
package cwg14;

class GameMap {

	private long startTime;
	private int tankId;
	private consoden.tankgame.GameState gameState;
	
	private byte empty=(byte)'.';
	
	
	public GameMap(int tankId, consoden.tankgame.GameState gameState) {
		this.startTime=System.nanoTime();
		this.tankId=tankId;
		this.gameState=gameState;
	}
	
	//X size of game field
	public int getSizeX() {
		return gameState.width().getVal();
	}
	
	//Y size of game field
	public int getSizeY() {
		return gameState.height().getVal();
	}
	
	//Is position an empty square. No walls or mines. Missiles and other tanks are not regarded here.
	public boolean isEmpty(final Position pos) {
		return gameState.board().getVal()[toIndex(pos)]==empty;
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

	//Is there a missile in this square
	public boolean IsMissileInPosition (Position p)
	{
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
	
	//Move p one step left. Handles wrap araound.
	public Position moveLeft(final Position p) {	
		return new Position((p.x-1+getSizeX())%getSizeX(), p.y);
	}
	
	//Move p one step right. Handles wrap araound.
	public Position moveRight(final Position p) {
		return new Position((p.x+1)%getSizeX(), p.y);
	}
	
	//Move p one step up. Handles wrap araound.
	public Position moveUp(final Position p) {
		return new Position(p.x, (p.y-1+getSizeY())%getSizeY());
	}
	
	//Move p one step down. Handles wrap araound.
	public Position moveDown(final Position p) {
		return new Position(p.x, (p.y+1)%getSizeY());
	}
	
	//Milliseconds elapsed since creation of this instance
	public long getElapsedTime() {
		return (System.nanoTime()-startTime)/1000000;
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
	        }
	        System.out.println();
	    }

	    System.out.println();
	}

	private int toIndex(int x, int y) {
		return x+y*getSizeY();
	}
	
	private int toIndex(final Position pos) {
		return pos.x+pos.y*getSizeX();
	}
}

class Position {
	public int x, y;
	public Position(int x, int y) {
		this.x=x;
		this.y=y;
	}
	
	public String toString() {
		return String.format("{%d, %d}", x, y);
	}
}

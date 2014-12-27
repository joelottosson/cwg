/******************************************************************************
*
* Copyright Consoden AB, 2014
*
* Created by: Joel Ottosson / joot
*
******************************************************************************/
package cwg14;


class TankLogic {
	
	public final static String PLAYER_NAME = "tank_player_java"; //TODO: Change to your team name
	
	public interface JoystickHandler {
		public void setJoystick(consoden.tankgame.Direction moveDirection,
								consoden.tankgame.Direction towerDirection,
								boolean fire, boolean dropMine);
	}
		
	private int tankId;
	private JoystickHandler joystickHandler;
	
	public TankLogic(int tankId, JoystickHandler joystickHandler){
		this.tankId=tankId;
		this.joystickHandler=joystickHandler;
	}
	
	public void makeMove(consoden.tankgame.GameState gameState) {
		
	    //TODO: implement your own tank logic and call setJoystick

		//-------------------------------------------------------
		//Example of a stupid tank logic:
		//Remove it and write your own brilliant version!
		//-------------------------------------------------------
	    GameMap gm=new GameMap(tankId, gameState); //helper object

		Position currentPosition = gm.getOwnPosition(); //this is our current tank position

		//Find an empty square we can move to, otherwize move downwards
		consoden.tankgame.Direction moveDirection = consoden.tankgame.Direction.NEUTRAL;
		
		if (!gm.isWall(gm.move(currentPosition, consoden.tankgame.Direction.LEFT)) &&
			!gm.isMine(gm.move(currentPosition, consoden.tankgame.Direction.LEFT))) {
			moveDirection=consoden.tankgame.Direction.LEFT;
		}
		else if (!gm.isWall(gm.move(currentPosition, consoden.tankgame.Direction.RIGHT)) &&
				 !gm.isMine(gm.move(currentPosition, consoden.tankgame.Direction.RIGHT))) {
			moveDirection=consoden.tankgame.Direction.RIGHT;
		}
		else if (!gm.isWall(gm.move(currentPosition, consoden.tankgame.Direction.UP)) &&
				 !gm.isMine(gm.move(currentPosition, consoden.tankgame.Direction.UP))) {
			moveDirection=consoden.tankgame.Direction.UP;
		}
		else if (!gm.isWall(gm.move(currentPosition, consoden.tankgame.Direction.DOWN)) &&
				 !gm.isMine(gm.move(currentPosition, consoden.tankgame.Direction.DOWN))) {
			moveDirection=consoden.tankgame.Direction.DOWN;
		}

		//Advanced tower aim stategy
		consoden.tankgame.Direction towerDirection = 
				consoden.tankgame.Direction.values()[(1 + currentPosition.x + currentPosition.y) % 4];			

		//Of course we always want to fire
		boolean fire = true;
		
		//Sometimes we also drop a mine
	    boolean dropMine=((gameState.elapsedTime().getVal().intValue()) % 3)==0;

		//Move our joystick.
		joystickHandler.setJoystick(moveDirection, towerDirection, fire, dropMine);
	}
	
	
}



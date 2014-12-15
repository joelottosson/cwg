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
								boolean fire);
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

		//Find an empty sqaure we can move to, otherwize move downwards
		consoden.tankgame.Direction moveDirection = consoden.tankgame.Direction.DOWN;
		if (gm.isEmpty(gm.moveLeft(currentPosition))) {
			moveDirection = consoden.tankgame.Direction.LEFT;
		} 
		else if (gm.isEmpty(gm.moveRight(currentPosition))) {
			moveDirection = consoden.tankgame.Direction.RIGHT;
		} 
		else if (gm.isEmpty(gm.moveUp(currentPosition))) {
			moveDirection = consoden.tankgame.Direction.UP;
		}

		//Advanced tower aim stategy
		consoden.tankgame.Direction towerDirection = 
				consoden.tankgame.Direction.values()[(currentPosition.x + currentPosition.y) % 4];			

		//Of course we always want to fire
		boolean fire = true;

		//Move our joystick.
		joystickHandler.setJoystick(moveDirection, towerDirection, fire);
	}
	
	
}



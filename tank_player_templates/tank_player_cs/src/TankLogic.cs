/******************************************************************************
*
* Copyright Consoden AB, 2014
*
* Created by: Joel Ottosson / joot
*
******************************************************************************/
using System;

namespace tank_player_cs
{
	delegate void SetJoystick (	Consoden.TankGame.Direction.Enumeration moveDirection,
	                            Consoden.TankGame.Direction.Enumeration towerDirection,
	                           	bool fire);

	//This class implements the tank logic.
	class TankLogic
	{
		//TODO: Change to your team name
		public const string PlayerName = "tank_player_csharp";

		private int tankId;
		private SetJoystick setJoystick;

		//Constructor: A new instance will be created every time a new game is started.
		public TankLogic (int tankId, SetJoystick setJoystick)
		{
			this.tankId=tankId;
			this.setJoystick=setJoystick;
		}

		//Called every time player is supposed to calculate a new move.
		//The player has 1 second to calculate the move and call setJoystick.
		public void MakeMove (Consoden.TankGame.GameState gameState)
		{
			//TODO: implement your own tank logic and call setJoystick

			//-------------------------------------------------------
			//Example of a stupid tank logic:
			//Remove it and write your own brilliant version!
			//-------------------------------------------------------
			GameMap gm = new GameMap (tankId, gameState); //helpler object

			Position currentPosition = gm.OwnPosition; //this is our current tank position

			//Find an empty sqaure we can move to, otherwize move downwards
			Consoden.TankGame.Direction.Enumeration moveDirection = Consoden.TankGame.Direction.Enumeration.Down;
			if (gm.IsEmpty (gm.MoveLeft (currentPosition))) {
				moveDirection = Consoden.TankGame.Direction.Enumeration.Left;
			} 
			else if (gm.IsEmpty (gm.MoveRight (currentPosition))) {
				moveDirection = Consoden.TankGame.Direction.Enumeration.Right;
			} 
			else if (gm.IsEmpty (gm.MoveUp (currentPosition))) {
				moveDirection = Consoden.TankGame.Direction.Enumeration.Up;
			}

			//Advanced tower aim stategy
			Consoden.TankGame.Direction.Enumeration towerDirection = 
				(Consoden.TankGame.Direction.Enumeration)((currentPosition.X + currentPosition.Y) % 4);

			//Of course we always want to fire
			bool fire = true;

			//Move our joystick.
			setJoystick (moveDirection, towerDirection, fire);
		}
	}
}


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
	                           	bool fire, bool dropMine);

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
			Bfs bfs=new Bfs(gameState, gm.OwnPosition); //breadth first search
			Consoden.TankGame.Direction.Enumeration moveDirection = Consoden.TankGame.Direction.Enumeration.Neutral;

			if (bfs.CanReachSquare (gm.EnemyPosition)) { //if we can reach the enemy, get him
				moveDirection = bfs.BacktrackFromSquare (gm.EnemyPosition);
			} 
			else { //find any empty square
				if (!gm.IsWall (gm.Move (gm.OwnPosition, Consoden.TankGame.Direction.Enumeration.Left)) &&
					!gm.IsMine (gm.Move (gm.OwnPosition, Consoden.TankGame.Direction.Enumeration.Left))) {
					moveDirection = Consoden.TankGame.Direction.Enumeration.Left;
				} else if (!gm.IsWall (gm.Move (gm.OwnPosition, Consoden.TankGame.Direction.Enumeration.Right)) &&
					!gm.IsMine (gm.Move (gm.OwnPosition, Consoden.TankGame.Direction.Enumeration.Right))) {
					moveDirection = Consoden.TankGame.Direction.Enumeration.Right;
				} else if (!gm.IsWall (gm.Move (gm.OwnPosition, Consoden.TankGame.Direction.Enumeration.Up)) &&
					!gm.IsMine (gm.Move (gm.OwnPosition, Consoden.TankGame.Direction.Enumeration.Up))) {
					moveDirection = Consoden.TankGame.Direction.Enumeration.Up;
				} else if (!gm.IsWall (gm.Move (gm.OwnPosition, Consoden.TankGame.Direction.Enumeration.Down)) &&
					!gm.IsMine (gm.Move (gm.OwnPosition, Consoden.TankGame.Direction.Enumeration.Down))) {
					moveDirection = Consoden.TankGame.Direction.Enumeration.Down;
				}
			}

			//Advanced tower aim stategy
			Consoden.TankGame.Direction.Enumeration towerDirection = 
				(Consoden.TankGame.Direction.Enumeration)((1 + gm.OwnPosition.X + gm.OwnPosition.Y) % 4);

			//Of course we always want to fire
			bool fire = true;

			//Sometimes we also drop a mine
			bool dropMine=((int)(gameState.ElapsedTime.Val) % 3)==0;

			//Move our joystick.
			setJoystick (moveDirection, towerDirection, fire, dropMine);
		}
	}
}


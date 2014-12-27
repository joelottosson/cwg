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
	struct Position
	{
		public int X;
		public int Y;

		Position(int x, int y)
		{
			X = x;
			Y = y;
		}
	}

	class GameMap
	{
		private int tankId;
		private Consoden.TankGame.GameState gameState;

		public GameMap (int tankId, Consoden.TankGame.GameState gameState)
		{
			this.tankId=tankId;
			this.gameState=gameState;
		}

		//X size of game field
		public int SizeX {
			get {
				return gameState.Width.Val;
			}
		}

		//Y size of game field
		public int SizeY {
			get {
				return gameState.Height.Val;
			}
		}

		//Own tank position
		public Position OwnPosition {
			get {
				Position p;
				p.X = gameState.Tanks [tankId].Obj.PosX.Val;
				p.Y = gameState.Tanks [tankId].Obj.PosY.Val;
				return p;
			}
		}

		//Enemy tank position
		public Position EnemyPosition {
			get {
				int i = (tankId + 1) % 2;
				Position p;
				p.X = gameState.Tanks [i].Obj.PosX.Val;
				p.Y = gameState.Tanks [i].Obj.PosY.Val;
				return p;
			}
		}

		//Check if square is a wall.
		public bool IsWall (Position p)
		{
			int i = ToIndex (p);
			return gameState.Board.Val [i] == Convert.ToByte ('x');
		}

		//Check if there is a mine in this square.
		public bool IsMine (Position p)
		{
			int i = ToIndex (p);
			return gameState.Board.Val [i] == Convert.ToByte ('o');
		}

		//Check if there is a coin in this square.
		public bool IsCoin (Position p)
		{
			int i = ToIndex (p);
			return gameState.Board.Val [i] == Convert.ToByte ('$');
		}

                //Check if there is poison gas in this square.
		public bool IsPoisonGas (Position p)
		{
			int i = ToIndex (p);
			return gameState.Board.Val [i] == Convert.ToByte ('p');
		}

		//Is there a missile in this square
		public bool IsMissileInPosition (Position p)
		{
			for (int i=0; i<this.gameState.Missiles.Count; i++) {
				if (gameState.Missiles [i].IsNull ())
					continue;

				Consoden.TankGame.Missile missile = gameState.Missiles [i].Obj;
				if ((p.X == missile.HeadPosX.Val && p.Y == missile.HeadPosY.Val) ||
					(p.X == missile.TailPosX.Val && p.Y == missile.TailPosY.Val)) {
					return true;
				}
			}
			return false;
		}

		//Move p one step in specified direction and returns the new position.
		public Position Move(Position p, Consoden.TankGame.Direction.Enumeration d)
		{
			switch (d) {
			case Consoden.TankGame.Direction.Enumeration.Left:
				{
				p.X = (p.X - 1 + SizeX) % SizeX;
				return p;
				}
			case Consoden.TankGame.Direction.Enumeration.Right:
				{
				p.X = (p.X + 1) % SizeX;
				return p;
				}
			case Consoden.TankGame.Direction.Enumeration.Up:
				{
				p.Y = (p.Y - 1 + SizeY) % SizeY;
				return p;
				}
			case Consoden.TankGame.Direction.Enumeration.Down:
				{
				p.Y = (p.Y + 1) % SizeY;
				return p;
				}
			default:
				return p;
			}
		}

		//Time left until the joystick will be readout next time.
		public int TimeToNextMove()
		{
			DateTime now = DateTime.Now;
			TimeSpan elapsedToday = now - new DateTime (now.Year, now.Month, now.Day);
			return gameState.NextMove.Val - (int)elapsedToday.TotalMilliseconds;
		}

		//Print game map
		public void PrintMap ()
		{
			for (int y=0; y<this.SizeY; y++) {
				for (int x=0; x<this.SizeX; x++) {
					int i = ToIndex (x, y);
					Console.Write (Convert.ToChar (gameState.Board.Val [i]));
				}
				Console.WriteLine ();
			}
			Console.WriteLine ();
		}

		//Print game state
		public void PrintState() 
		{
		    Console.WriteLine ("Game board");
		    PrintMap();

		    Console.WriteLine ("Own position {0},{1}", OwnPosition.X, OwnPosition.Y);
		    Console.WriteLine ("Enemy position {0},{1}", EnemyPosition.X, EnemyPosition.Y);

		    Console.WriteLine ("Active missiles");

			for (int i=0; i<this.gameState.Missiles.Count; i++) {
				if (gameState.Missiles [i].IsNull ())
					continue;

				Consoden.TankGame.Missile missile = gameState.Missiles [i].Obj;

				Position head;
				head.X = missile.HeadPosX.Val;
				head.Y = missile.HeadPosY.Val;
				Position tail;
				tail.X = missile.TailPosX.Val;
				tail.Y = missile.TailPosY.Val;
		        Consoden.TankGame.Direction.Enumeration direction = missile.Direction.Val;

		        Console.WriteLine ("Missile position - head {0},{1}", head.X, head.Y);
		        Console.WriteLine ("Missile position - tail {0},{1}", tail.X, tail.Y);
		        Console.Write ("Missile direction ");

		        switch (direction) {
		        case Consoden.TankGame.Direction.Enumeration.Left:
		            Console.WriteLine ("Left");
		            break;
		        case Consoden.TankGame.Direction.Enumeration.Right:
		            Console.WriteLine ("Right");
		            break;
		        case Consoden.TankGame.Direction.Enumeration.Up:
		            Console.WriteLine ("Up");
		            break;
		        case Consoden.TankGame.Direction.Enumeration.Down:
		            Console.WriteLine ("Down");
		            break;
                        case Consoden.TankGame.Direction.Enumeration.Neutral:
                            Console.WriteLine ("Neutral");
                            break;
		        }
		        Console.WriteLine ();
		    }

		    Console.WriteLine ();
		}

		private int ToIndex (int x, int y)
		{
			return x + y * SizeX;
		}

		private int ToIndex(Position p)
		{
			return ToIndex(p.X, p.Y);
		}

	}
}

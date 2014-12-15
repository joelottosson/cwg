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
	}

	class GameMap
	{
		private DateTime startTime = DateTime.Now;
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

		//Is position an empty square. No walls or mines. Missiles and other tanks are not regarded here.
		public bool IsEmpty (Position p)
		{
			int i = ToIndex (p);
			return gameState.Board.Val [i] == Convert.ToByte ('.');
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

		//Move p one step left. Handles wrap araound.
		public Position MoveLeft (Position p)
		{
			p.X = (p.X - 1 + SizeX) % SizeX;
			return p;
		}

		//Move p one step right. Handles wrap araound.
		public Position MoveRight (Position p)
		{
			p.X = (p.X + 1) % SizeX;
			return p;
		}

		//Move p one step up. Handles wrap araound.
		public Position MoveUp (Position p)
		{
			p.Y = (p.Y - 1 + SizeY) % SizeY;
			return p;
		}

		//Move p one step down. Handles wrap araound.
		public Position MoveDown (Position p)
		{
			p.Y = (p.Y + 1) % SizeY;
			return p;
		}

		//Milliseconds elapsed since creation of this instance
		public long Elapsed {
			get {
				return (int)((DateTime.Now - startTime).TotalMilliseconds);
			}
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

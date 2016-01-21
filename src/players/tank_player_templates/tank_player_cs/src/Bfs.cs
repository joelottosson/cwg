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
	//Implementation of a breadth first algorithm
	//Helpful to find out if a square is reachable and get the shortest path.
	class Bfs
	{
		private Consoden.TankGame.GameState gameState;
		private int[,] gamePaths = null;
		private Position startPos;
		private bool allowGas = true;

		//Create a bfs graph based on startPos.
		public Bfs (Consoden.TankGame.GameState gameState, Position startPos, bool allowGas = true)
		{
			this.gameState=gameState;
			this.startPos = startPos;
			this.allowGas = allowGas;

			gamePaths = new int[SizeX, SizeY];
			for (int x=0; x<SizeX; x++) {
				for (int y=0; y<SizeY; y++) {
					gamePaths [x, y] = int.MaxValue;
				}
			}

			gamePaths [startPos.X, startPos.Y] = 0;
			EvaluateShortestPaths (startPos, 1);
		}	

		//Check if square is reachable.
		public bool CanReachSquare(Position pos)
		{
			if (pos.X >= SizeX || pos.X < 0 || pos.Y >= SizeY || pos.Y < 0) {
				return false;
			}
			return gamePaths[pos.X, pos.Y] < int.MaxValue;
		}

		//Least number of moves needed to reach square.
		public int StepsToSquare(Position pos)
		{
			if (pos.X >= SizeX || pos.X < 0 || pos.Y >= SizeY || pos.Y < 0) {
				return int.MaxValue;
			}
			return gamePaths[pos.X, pos.Y];
		}

		//Find the direction to move for shortest path to target.
		public Consoden.TankGame.Direction.Enumeration BacktrackFromSquare(Position target)
		{
			var nextMove = Backtrack (target);

			// Shortest path is one step less
			if (Move (startPos, Consoden.TankGame.Direction.Enumeration.Left) == nextMove)
				return Consoden.TankGame.Direction.Enumeration.Left;
			else if (Move (startPos, Consoden.TankGame.Direction.Enumeration.Right) == nextMove)
				return Consoden.TankGame.Direction.Enumeration.Right;
			else if (Move (startPos, Consoden.TankGame.Direction.Enumeration.Up) == nextMove)
				return Consoden.TankGame.Direction.Enumeration.Up;
			else if (Move (startPos, Consoden.TankGame.Direction.Enumeration.Down) == nextMove)
				return Consoden.TankGame.Direction.Enumeration.Down;
			else
				return Consoden.TankGame.Direction.Enumeration.Neutral;
		}


		private Position Backtrack(Position target)
		{
			if (!CanReachSquare(target)) {
				return new Position (-1, -1);
			}

			// Steps to square
			int steps = StepsToSquare(target);

			// one step - found the next square
			if (steps == 1) {
				return target;
			}

			// Shortest path is one step less
			var left = Move (target, Consoden.TankGame.Direction.Enumeration.Left);
			if (StepsToSquare(left) == (steps-1)) {
				return Backtrack(left);
			}

			var right = Move (target, Consoden.TankGame.Direction.Enumeration.Right);
			if (StepsToSquare(right) == (steps-1)) {
				return Backtrack(right);
			}

			var up = Move (target, Consoden.TankGame.Direction.Enumeration.Up);
			if (StepsToSquare(up) == (steps-1)) {
				return Backtrack(up);
			}

			var down = Move (target, Consoden.TankGame.Direction.Enumeration.Down);
			if (StepsToSquare(down) == (steps-1)) {
				return Backtrack(down);
			}

			// How did this happend??
			return new Position (-1, -1);	
		}

		private void EvaluateShortestPathsHelper(Position nextPos, int steps) 
		{
			char next = RawVal (nextPos.X, nextPos.Y);
			bool isMine = next == 'o';
			bool isWall = next == 'x';
			bool isPoison = next == 'p';

			if (!(isMine || isWall) && (allowGas || !isPoison)) 
			{
				if (steps < gamePaths[nextPos.X, nextPos.Y]) {
					gamePaths[nextPos.X, nextPos.Y] = steps;
					EvaluateShortestPaths(nextPos, steps+1);                
				}
			}
		}

		private void EvaluateShortestPaths(Position pos, int steps)
		{
			EvaluateShortestPathsHelper(Move(pos, Consoden.TankGame.Direction.Enumeration.Left), steps);
			EvaluateShortestPathsHelper(Move(pos, Consoden.TankGame.Direction.Enumeration.Right), steps);
			EvaluateShortestPathsHelper(Move(pos, Consoden.TankGame.Direction.Enumeration.Up), steps);
			EvaluateShortestPathsHelper(Move(pos, Consoden.TankGame.Direction.Enumeration.Down), steps);
		}

		//X size of game field
		private int SizeX {
			get {
				return gameState.Width.Val;
			}
		}

		//Y size of game field
		private int SizeY {
			get {
				return gameState.Height.Val;
			}
		}

		private int ToIndex (int x, int y)
		{
			return x + y * SizeX;
		}

		private char RawVal(int x, int y)
		{
			int i = ToIndex (x, y);
			return Convert.ToChar (gameState.Board.Val [i]);
		}

		//Move p one step in specified direction and returns the new position.
		public Position Move(Position p, Consoden.TankGame.Direction.Enumeration d)
		{
			switch (d) {
				case Consoden.TankGame.Direction.Enumeration.Left:
				return new Position ((p.X - 1 + SizeX) % SizeX, p.Y);
				case Consoden.TankGame.Direction.Enumeration.Right:
				return new Position ((p.X + 1) % SizeX, p.Y);
				case Consoden.TankGame.Direction.Enumeration.Up:
				return new Position (p.X, (p.Y - 1 + SizeY) % SizeY);
				case Consoden.TankGame.Direction.Enumeration.Down:
				return new Position (p.X, (p.Y + 1) % SizeY);
				default:
				return p;
			}
		}
	}
}

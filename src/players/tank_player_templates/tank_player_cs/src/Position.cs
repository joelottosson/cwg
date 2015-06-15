/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
******************************************************************************/
using System;

namespace tank_player_cs
{
	//Represents a position or square in the game.
	//Immutable value type.
	struct Position
	{
		//Position coordinates.
		public readonly int X;
		public readonly int Y;

		//Constuctor, creates a new position.
		public Position(int x, int y)
		{
			X = x;
			Y = y;
		}

		//Equality operator .
		public static bool operator ==(Position a, Position b)
		{
			return a.Equals (b);
		}

		//Inequality operator.
		public static bool operator !=(Position a, Position b)
		{
			return !(a.Equals (b));
		}

		//Get a readable string representation of the position.
		public override string ToString ()
		{
			return string.Format ("({0}, {1})", X, Y);
		}

		//Compare this position to another.
		public override bool Equals (object obj)
		{
			if (obj is Position) {
				Position p = (Position)obj;
				return X == p.X && Y == p.Y;
			}
			return false;
		}

		public override int GetHashCode ()
		{
			return base.GetHashCode ();
		}
	}
}

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
		public readonly int X;
		public readonly int Y;

		public Position(int x, int y)
		{
			X = x;
			Y = y;
		}

		public static bool operator ==(Position a, Position b)
		{
			return a.Equals (b);
		}

		public static bool operator !=(Position a, Position b)
		{
			return !(a.Equals (b));
		}

		public override string ToString ()
		{
			return string.Format ("({0}, {1})", X, Y);
		}

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

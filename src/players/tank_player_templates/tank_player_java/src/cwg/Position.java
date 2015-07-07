/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
******************************************************************************/
package cwg;

//Represents a position or square in the game
class Position {
	public int x, y; //The coordinates.
	
	//Constuctor, creates a new position.
	public Position(int x, int y) {
		this.x=x;
		this.y=y;
	}
	
	//Get a readable string representation of the position.
	public String toString() {
		return String.format("{%d, %d}", x, y);
	}
	
	//Compare this position to another.
	public boolean equals(Object object) {
		if(object instanceof Position) {
			Position p=(Position)object;
			return x==p.x && y==p.y;
		}
		return false;
	}
}

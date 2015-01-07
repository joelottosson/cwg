/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
******************************************************************************/
package cwg;

class Position {
	public int x, y;
	
	public Position(int x, int y) {
		this.x=x;
		this.y=y;
	}
	
	public String toString() {
		return String.format("{%d, %d}", x, y);
	}
	
	public boolean equals(Object object) {
		if(object instanceof Position) {
			Position p=(Position)object;
			return x==p.x && y==p.y;
		}
		return false;
	}
}

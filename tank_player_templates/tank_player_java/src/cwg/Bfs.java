/******************************************************************************
*
* Copyright Consoden AB, 2014
*
* Created by: Joel Ottosson / joot
*
******************************************************************************/
package cwg;

//Implementation of a breadth first algorithm
//Helpful to find out if a square is reachable and get the shortest path.
class Bfs {
	
	private consoden.tankgame.GameState gameState;	
	private Position startPos;
	private int[][] gamePaths = null;
	
	private byte empty=(byte)'.';
	private byte coin=(byte)'$';
	private byte poison=(byte)'p';
	
	//Create a bfs graph based on startPos.
	public Bfs(consoden.tankgame.GameState gameState, Position startPos) {		
		this.gameState=gameState;
		this.startPos=startPos;
		gamePaths=new int[getSizeX()][getSizeY()];
		for (int x=0; x<getSizeX(); x++) {
			for (int y=0; y<getSizeY(); y++) {
				gamePaths[x][y] = Integer.MAX_VALUE;
			}
		}

		gamePaths[startPos.x][startPos.y] = 0;
					
		evaluateShortestPaths (startPos, 1);
	}
	
	//Check if square is reachable.
	public boolean canReachSquare(Position pos) {
		if (pos.x >= getSizeX() || pos.x < 0 || pos.y >= getSizeY() || pos.y < 0) {
			return false;
		}
		return gamePaths[pos.x][pos.y] < Integer.MAX_VALUE;
	}

	//Least number of moves needed to reach square.
	public int stepsToSquare(Position pos) {
		if (pos.x >= getSizeX() || pos.x < 0 || pos.y >= getSizeY() || pos.y < 0) {
			return Integer.MAX_VALUE;
		}
		return gamePaths[pos.x][pos.y];
	}
	
	//Find the direction to move for shortest path to target.
	public consoden.tankgame.Direction backtrackFromSquare(Position target) {
		Position nextMove = backtrack (target);

		// Shortest path is one step less
		if (nextMove.equals(move(startPos, consoden.tankgame.Direction.LEFT)))
			return consoden.tankgame.Direction.LEFT;
		else if (nextMove.equals(move(startPos, consoden.tankgame.Direction.RIGHT)))
			return consoden.tankgame.Direction.RIGHT;
		else if (nextMove.equals(move(startPos, consoden.tankgame.Direction.UP)))
			return consoden.tankgame.Direction.UP;
		else if (nextMove.equals(move(startPos, consoden.tankgame.Direction.DOWN)))
			return consoden.tankgame.Direction.DOWN;
		else
			return consoden.tankgame.Direction.NEUTRAL;
	}
	
	private Position backtrack(Position target) {
		if (!canReachSquare(target)) {
			return new Position (-1, -1);
		}

		// Steps to square
		int steps = stepsToSquare(target);

		// one step - found the next square
		if (steps == 1) {
			return target;
		}

		// Shortest path is one step less
		Position left = move(target, consoden.tankgame.Direction.LEFT);
		if (stepsToSquare(left) == (steps-1)) {
			return backtrack(left);
		}

		Position right = move(target, consoden.tankgame.Direction.RIGHT);
		if (stepsToSquare(right) == (steps-1)) {
			return backtrack(right);
		}

		Position up = move(target, consoden.tankgame.Direction.UP);
		if (stepsToSquare(up) == (steps-1)) {
			return backtrack(up);
		}

		Position down = move(target, consoden.tankgame.Direction.DOWN);
		if (stepsToSquare(down) == (steps-1)) {
			return backtrack(down);
		}

		// How did this happend??
		return new Position (-1, -1);	
	}
	
	//X size of game field
	private int getSizeX() {
		return gameState.width().getVal();
	}
	
	//Y size of game field
	private int getSizeY() {
		return gameState.height().getVal();
	}
		
	private Position move(final Position p, final consoden.tankgame.Direction d) {
		switch (d) {
        case LEFT:
        	return new Position((p.x-1+getSizeX())%getSizeX(), p.y);
        case RIGHT:
        	return new Position((p.x+1)%getSizeX(), p.y);
        case UP:
        	return new Position(p.x, (p.y-1+getSizeY())%getSizeY());
        case DOWN:
        	return new Position(p.x, (p.y+1)%getSizeY());
        case NEUTRAL:
        	return new Position(p.x, p.y);
        }
		return new Position(p.x, p.y);
	}

	private int toIndex(int x, int y) {
		return x+y*getSizeX();
	}
	
	private byte rawVal(int x, int y) {
		return gameState.board().getVal()[toIndex(x, y)];
	}
	
	private void evaluateShortestPathsHelper(Position nextPos, int steps) {		
		byte next = rawVal(nextPos.x, nextPos.y);
		boolean isEmpty= next==empty || next==coin || next==poison;
		if (isEmpty) 
		{
			if (steps < gamePaths[nextPos.x][nextPos.y]) {
				System.out.println(nextPos.toString()+" evaluated step: "+steps);
				gamePaths[nextPos.x][nextPos.y] = steps;
				evaluateShortestPaths(nextPos, steps+1);                
			}
		}
		else
		{
			System.out.println(nextPos.toString()+" not empty: "+(char)next);
		}
	}

	private void evaluateShortestPaths(Position pos, int steps) {
		evaluateShortestPathsHelper(move(pos, consoden.tankgame.Direction.LEFT), steps);
		evaluateShortestPathsHelper(move(pos, consoden.tankgame.Direction.RIGHT), steps);
		evaluateShortestPathsHelper(move(pos, consoden.tankgame.Direction.UP), steps);
		evaluateShortestPathsHelper(move(pos, consoden.tankgame.Direction.DOWN), steps);
	}
	
	/*private void dump() {
		System.out.println("--- path ---");
		for (int y=0; y<getSizeY(); y++) {
			for (int x=0; x<getSizeX(); x++) {
				if (gamePaths[x][y]<Integer.MAX_VALUE)
					System.out.print(gamePaths[x][y]);
				else
					System.out.print("X");
				System.out.print(" ");
			}
			System.out.println();
		}
	}*/
}

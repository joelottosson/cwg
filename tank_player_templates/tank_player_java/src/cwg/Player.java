/******************************************************************************
*
* Copyright Consoden AB, 2014
*
* Created by: Joel Ottosson / joot
*
******************************************************************************/
package cwg;

import com.saabgroup.safir.dob.typesystem.*;
import com.saabgroup.safir.dob.ErrorResponse;
import com.saabgroup.safir.dob.ResponseGeneralErrorCodes;
import consoden.tankgame.Direction;

class Player implements com.saabgroup.safir.dob.Dispatcher,
                    com.saabgroup.safir.dob.StopHandler,
                    com.saabgroup.safir.dob.EntityHandler,
                    com.saabgroup.safir.dob.EntitySubscriber,
                    TankLogic.JoystickHandler {
    
	private boolean running=true;
    private com.saabgroup.safir.dob.Connection dobConnection;
    private boolean dispatchSignal=false;
    private InstanceId currentGameId = null;
    private HandlerId myHandlerId = null;
    private InstanceId myPlayerId = null;
    private InstanceId myJoystickId = null;
    private int myTankId = -1;
    private TankLogic logic = null;
    private int joystickCounter = 0;
        
    public Player() {
    	//set up our unique id's
        myHandlerId=new HandlerId(TankLogic.PLAYER_NAME+"Handler");
        myPlayerId= new InstanceId(TankLogic.PLAYER_NAME+"Instance");
        
        //open DOB connection. Register player and joystick. Subscribe for gameStates.
        dobConnection = new com.saabgroup.safir.dob.Connection();
        dobConnection.open(TankLogic.PLAYER_NAME, "", 0, this, this);
        dobConnection.registerEntityHandler(consoden.tankgame.Player.ClassTypeId, myHandlerId, com.saabgroup.safir.dob.InstanceIdPolicy.HANDLER_DECIDES_INSTANCE_ID, this);
        dobConnection.registerEntityHandler(consoden.tankgame.Joystick.ClassTypeId, myHandlerId, com.saabgroup.safir.dob.InstanceIdPolicy.HANDLER_DECIDES_INSTANCE_ID, this);
        dobConnection.subscribeEntity(consoden.tankgame.GameState.ClassTypeId, this);
        
        //create our player entity
        consoden.tankgame.Player player=new consoden.tankgame.Player();
        player.name().setVal(TankLogic.PLAYER_NAME);
        dobConnection.setAll(player, myPlayerId, myHandlerId);

        //run the game player
        run();
    }

    @Override
    public void onRevokedRegistration(long typeId, com.saabgroup.safir.dob.typesystem.HandlerId handlerId) {
    	
    	if (handlerId==myHandlerId) {
    		System.out.println("Revoked!");
    		onStopOrder();
    	}
    }
    
    @Override
    public void onCreateRequest(com.saabgroup.safir.dob.EntityRequestProxy entityRequestProxy, 
            com.saabgroup.safir.dob.ResponseSender responseSender) {
            
    	responseSender.send(ErrorResponse.createErrorResponse(ResponseGeneralErrorCodes.getSafirNoPermission(), ""));
    }

    @Override
    public void onDeleteRequest(com.saabgroup.safir.dob.EntityRequestProxy entityRequestProxy, 
            com.saabgroup.safir.dob.ResponseSender responseSender) {
    	
    	responseSender.send(ErrorResponse.createErrorResponse(ResponseGeneralErrorCodes.getSafirNoPermission(), ""));
    }

    @Override
    public void onUpdateRequest(com.saabgroup.safir.dob.EntityRequestProxy entityRequestProxy, 
            com.saabgroup.safir.dob.ResponseSender responseSender) {
            
    	responseSender.send(ErrorResponse.createErrorResponse(ResponseGeneralErrorCodes.getSafirNoPermission(), ""));
    }
    
    @Override
    public void onNewEntity(com.saabgroup.safir.dob.EntityProxy entityProxy) {

    	if (entityProxy.getEntity() instanceof consoden.tankgame.GameState) {
    			
    		consoden.tankgame.GameState gameState=(consoden.tankgame.GameState)entityProxy.getEntity();
    		
    		for (int i=0; i<gameState.tanks().size(); i++) {
    			if (!gameState.tanks().get(i).isNull()) {
    				consoden.tankgame.Tank tank=gameState.tanks().get(i).getObj();
    				if (tank.playerId().getVal().equals(myPlayerId)) {
    					this.currentGameId=entityProxy.getInstanceId();
    					this.myTankId=tank.tankId().getVal();
    					this.myJoystickId=InstanceId.generateRandom();
    					
    					consoden.tankgame.Joystick joystick=new consoden.tankgame.Joystick();
    					joystickCounter=0;
    					joystick.playerId().setVal(myPlayerId);
    					joystick.gameId().setVal(currentGameId);
    					joystick.tankId().setVal(myTankId);
    					joystick.counter().setVal(joystickCounter++);
    					dobConnection.setAll(joystick, myJoystickId, myHandlerId);
    					logic=new TankLogic(myTankId, this);
    					break;
    				}
    			}
    		}
    	}
    }
    
    @Override
    public void onUpdatedEntity(com.saabgroup.safir.dob.EntityProxy entityProxy) {
    	if (entityProxy.getEntity() instanceof consoden.tankgame.GameState &&
    			entityProxy.getInstanceId().equals(currentGameId) && logic!=null) {
            
            consoden.tankgame.GameState gs = (consoden.tankgame.GameState)entityProxy.getEntity();
            if (gs.winner().getVal() == consoden.tankgame.Winner.UNKNOWN) {
                try {
				    logic.makeMove((consoden.tankgame.GameState)entityProxy.getEntity());
			    } 
			    catch (java.lang.Exception e) {
				    System.out.println("Caught unhandled exception in TankLogic!");
			    }
                
            }
    	}
    }
    
    @Override
    public void onDeletedEntity(com.saabgroup.safir.dob.EntityProxy entityProxy, boolean deprecated) {
    	if (entityProxy.getTypeId()==consoden.tankgame.GameState.ClassTypeId && 
    			entityProxy.getInstanceId().equals(currentGameId)) {
    		if (myJoystickId!=null) {
    			dobConnection.delete(new EntityId(consoden.tankgame.Joystick.ClassTypeId, myJoystickId), myHandlerId);
    		}
    		
    		logic=null;            
            myJoystickId=null;
            currentGameId=null;
            myTankId=-1;
        }
    }

    @Override
    public void onStopOrder() {
    	synchronized(this) {
            running=false;
            this.notify();
        }
    }
    
    @Override
    public void onDoDispatch() {
        synchronized(this) {
            dispatchSignal=true;
            this.notify();
        }
    }
    
    private void run() {
    	System.out.println(TankLogic.PLAYER_NAME+" is running");
    	while (running) {
	        synchronized(this) {
	            while(!dispatchSignal){
	            	if (!running)
	            		return;
	            	
	                try {
	                    this.wait();
	                } catch(InterruptedException e){}
	            }

	            dispatchSignal=false;
	            dobConnection.dispatch();
	        }
    	}
    }
    
	@Override
	public void setJoystick(Direction moveDirection, Direction towerDirection, boolean fire, boolean dropMine) {

	    if (myJoystickId==null) {
	        return; //we are not active in a game
	    }
	    
	    if (moveDirection==null)
	        moveDirection=consoden.tankgame.Direction.NEUTRAL;
        if (towerDirection==null)
	        towerDirection=consoden.tankgame.Direction.NEUTRAL;

	    consoden.tankgame.Joystick joystick=new consoden.tankgame.Joystick();
		joystick.playerId().setVal(myPlayerId);
		joystick.gameId().setVal(currentGameId);
		joystick.tankId().setVal(myTankId);
		joystick.counter().setVal(joystickCounter++);
		joystick.moveDirection().setVal(moveDirection);
		joystick.towerDirection().setVal(towerDirection);
		joystick.fire().setVal(fire);
		joystick.mineDrop().setVal(dropMine);
		dobConnection.setAll(joystick, myJoystickId, myHandlerId);
	}
	
	public static void main(String[] args) {
        new Player();
    }
}

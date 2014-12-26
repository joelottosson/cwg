/******************************************************************************
*
* Copyright Consoden AB, 2014
*
* Created by: Joel Ottosson / joot
*
******************************************************************************/
using System;
using System.Threading;
using Safir.Dob.Typesystem;

namespace tank_player_cs
{
	class Player : 	Safir.Dob.Dispatcher,
					Safir.Dob.StopHandler,
					Safir.Dob.EntityHandler,
					Safir.Dob.EntitySubscriber
	{
		private AutoResetEvent dispatchEvent=new AutoResetEvent(false);
		private bool running=true;
		private Safir.Dob.Connection connection=new Safir.Dob.Connection();
		private TankLogic logic=null;

		private HandlerId myHandlerId=null;
		private InstanceId currentGameId=null;
		private InstanceId myPlayerId=null;
		private InstanceId myJoystickId=null;
		private int myTankId=-1;
		private int joystickCounter=0;

		public Player ()
		{
			//This players unique identifiers
			myHandlerId = new HandlerId (TankLogic.PlayerName + "Handler");
			myPlayerId = new InstanceId (TankLogic.PlayerName + "Instance");

			// Open DOB connection. Register player and joystick, subscribe for gameStates
			connection.Open (TankLogic.PlayerName, "", 0, this, this);
			connection.RegisterEntityHandler (Consoden.TankGame.Player.ClassTypeId, myHandlerId, Safir.Dob.InstanceIdPolicy.Enumeration.HandlerDecidesInstanceId, this);
			connection.RegisterEntityHandler (Consoden.TankGame.Joystick.ClassTypeId, myHandlerId, Safir.Dob.InstanceIdPolicy.Enumeration.HandlerDecidesInstanceId, this);
			connection.SubscribeEntity (Consoden.TankGame.GameState.ClassTypeId, this);
	        
			//Create our player entity
			Consoden.TankGame.Player player = new Consoden.TankGame.Player ();
			player.Name.Val = TankLogic.PlayerName;	        
			connection.SetAll (player, myPlayerId, myHandlerId);

			//Run the game player
			Run ();
		}

		public void OnDoDispatch ()
		{
			dispatchEvent.Set (); //signal to main thread
		}

		public void OnStopOrder ()
		{
			running = false;
			dispatchEvent.Set ();
		}

		public void OnRevokedRegistration (long typeId, Safir.Dob.Typesystem.HandlerId handlerId)
		{
			System.Console.WriteLine("Revoked by handler "+handlerId.ToString());
		}

		public void OnCreateRequest (Safir.Dob.EntityRequestProxy entityRequestProxy, Safir.Dob.ResponseSender responseSender)
		{
			responseSender.Send (Safir.Dob.ErrorResponse.CreateErrorResponse (Safir.Dob.ResponseGeneralErrorCodes.SafirNoPermission, ""));
		}

		public void OnUpdateRequest (Safir.Dob.EntityRequestProxy entityRequestProxy, Safir.Dob.ResponseSender responseSender)
		{
			responseSender.Send (Safir.Dob.ErrorResponse.CreateErrorResponse (Safir.Dob.ResponseGeneralErrorCodes.SafirNoPermission, ""));
		}

		public void OnDeleteRequest (Safir.Dob.EntityRequestProxy entityRequestProxy, Safir.Dob.ResponseSender responseSender)
		{
			responseSender.Send (Safir.Dob.ErrorResponse.CreateErrorResponse (Safir.Dob.ResponseGeneralErrorCodes.SafirNoPermission, ""));
		}

		public void OnNewEntity (Safir.Dob.EntityProxy entityProxy)
		{
			Consoden.TankGame.GameState gameState = entityProxy.Entity as Consoden.TankGame.GameState;
			if (gameState == null) {
				return;
			}

			for (int i=0; i<gameState.Tanks.Count; i++) {
				if (!gameState.Tanks [i].IsNull ()) {
					Consoden.TankGame.Tank tank = gameState.Tanks [i].Obj;
					if (tank.PlayerId.Val == myPlayerId) {
						currentGameId = entityProxy.InstanceId;
						myTankId = tank.TankId.Val;
						myJoystickId = InstanceId.GenerateRandom ();

						Consoden.TankGame.Joystick joystick = new Consoden.TankGame.Joystick ();
						joystickCounter = 0;
						joystick.PlayerId.Val = myPlayerId;
						joystick.GameId.Val = currentGameId;
						joystick.TankId.Val = myTankId;
						joystick.Counter.Val = joystickCounter++;
						connection.SetAll (joystick, myJoystickId, myHandlerId);
						logic = new TankLogic (myTankId, UpdateJoystick);
						try
						{
						    logic.MakeMove (gameState);
					    }
					    catch
					    {
					        Console.WriteLine("Caught unhandled exception in TankLogic!");
					    }
						break;
					}
				}
			}
		}

		public void OnUpdatedEntity (Safir.Dob.EntityProxy entityProxy)
		{
			if (entityProxy.TypeId == Consoden.TankGame.GameState.ClassTypeId &&
				entityProxy.InstanceId == currentGameId && logic!=null) {
				Consoden.TankGame.GameState gs = (Consoden.TankGame.GameState)entityProxy.Entity;
				if (gs.Winner.Val == Consoden.TankGame.Winner.Enumeration.Unknown) {
				    try
					{
					    logic.MakeMove((Consoden.TankGame.GameState)entityProxy.Entity);
				    }
				    catch
				    {
				        Console.WriteLine("Caught unhandled exception in TankLogic!");
				    }
				}
			}
		}

		public void OnDeletedEntity (Safir.Dob.EntityProxy entityProxy, bool deprecated)
		{
			if (entityProxy.TypeId == Consoden.TankGame.GameState.ClassTypeId &&
				entityProxy.InstanceId == currentGameId) {
				if (myJoystickId != null) {
					connection.Delete (new EntityId (Consoden.TankGame.Joystick.ClassTypeId, myJoystickId), myHandlerId);
				}

				logic = null;
				myJoystickId = null;
				currentGameId = null;
				myTankId = -1;
			}
		}

		private void UpdateJoystick (Consoden.TankGame.Direction.Enumeration moveDirection,
		                            Consoden.TankGame.Direction.Enumeration towerDirection,
		                            bool fire, bool dropMine)
		{
			if (myJoystickId == null) {
				return; //we are not active in a game
			}

			Consoden.TankGame.Joystick joystick = new Consoden.TankGame.Joystick ();
			joystick.PlayerId.Val = myPlayerId;
			joystick.GameId.Val = currentGameId;
			joystick.TankId.Val = myTankId;
			joystick.Counter.Val = joystickCounter++;
			joystick.MoveDirection.Val = moveDirection;
			joystick.TowerDirection.Val = towerDirection;
			joystick.Fire.Val = fire;
			joystick.MineDrop.Val = dropMine;
			connection.SetAll (joystick, myJoystickId, myHandlerId);
		}

		private void Run ()
		{
			Console.WriteLine (TankLogic.PlayerName + " is running");
			while (running) {
				dispatchEvent.WaitOne ();
				connection.Dispatch();
			}
		}

		public static void Main (string[] args)
		{
			new Player();
		}
	}
}

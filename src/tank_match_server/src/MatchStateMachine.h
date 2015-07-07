/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#pragma once
#include <vector>
#include <functional>
#include <Consoden/TankGame/Match.h>
#include <Consoden/TankGame/GameState.h>

namespace sd = Safir::Dob;
namespace sdt = Safir::Dob::Typesystem;
namespace cwg = Consoden::TankGame;

class MatchStateMachine
{
public:
    MatchStateMachine(cwg::MatchPtr matchRequest,
               std::function<void(cwg::GameStatePtr)> onStartNewGame,
               std::function<void()> onUpdateMatchState);

    void Start();
    void Reset();

    //Update state machine
    void OnNewGameState(const cwg::GameStatePtr gameState);
    void OnUpdatedGameState(const cwg::GameStatePtr gameState);

    cwg::MatchPtr CurrentState() const {return m_state;}
    sdt::EntityId MatchEntityId() const {return m_matchEid;}
    int hashBoard(std::vector<char> board) const;

private:
    cwg::MatchPtr m_state;
    sdt::EntityId m_matchEid;
    std::function<void (cwg::GameStatePtr)> m_onStartNewGame;
    std::function<void ()> m_onUpdateMatchState;
    std::vector<cwg::GameStatePtr> m_games;
    int m_repetion;
    cwg::GameStatePtr m_currentGameState;
    bool m_running;
    bool m_currentGameStateHasFinished;
    int m_player1PointsAccumulated;
    int m_player2PointsAccumulated;

    void CreateBoards();
    cwg::GameStatePtr CreateGameState(const std::string& boardFile, bool reversedPlayers) const;
    void StartNextGame();    
    void UpdatePoints();
    bool HasGameFinished() const;
    bool HasMatchFinished() const;
    void HandleMatchFinished();
};

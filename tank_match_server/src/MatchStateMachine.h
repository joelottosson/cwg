/******************************************************************************
*
* Copyright Consoden AB, 2014
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
               std::function<void()> onMatchFinished);

    void Start();
    void Reset();

    //Update state machine
    void OnNewGameState(const cwg::GameStatePtr gameState);
    void OnUpdatedGameState(const cwg::GameStatePtr gameState);

    cwg::MatchPtr CurrentState() const {return m_state;}
    sdt::EntityId MatchEntityId() const {return m_matchEid;}

private:
    cwg::MatchPtr m_state;
    sdt::EntityId m_matchEid;
    std::function<void (cwg::GameStatePtr)> m_onStartNewGame;
    std::function<void ()> m_onMatchFinished;
    std::vector<cwg::GameStatePtr> m_games;
    int m_repetion;
    cwg::GameStatePtr m_lastGameState;
    bool m_running;
    bool m_currentGameStateHasFinished;

    void CreateBoards();
    cwg::GameStatePtr CreateGameState(const std::string& boardFile, bool reversedPlayers) const;
    void StartNextGame();
    void UpdatePoints(const cwg::GameStatePtr& updatedState);
    bool HasGameFinished(const cwg::GameStatePtr& game) const;
    bool HasMatchFinished() const;
    void HandleMatchFinished();
};

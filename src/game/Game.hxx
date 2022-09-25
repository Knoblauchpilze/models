#ifndef    GAME_HXX
# define   GAME_HXX

# include "Game.hh"

namespace pge {

  inline
  void
  Game::terminate() noexcept {
    info("Game has been terminated");
    m_state.terminated = true;
  }

  inline
  bool
  Game::terminated() const noexcept {
    return m_state.terminated;
  }

}

#endif    /* GAME_HXX */


# include "Model.hh"

namespace eqdif {

  Model::Model():
    utils::CoreObject("model"),
    Process()
  {
    setService("eqdif");
  }

  void
  Model::load(const std::string& file) {
    /// TODO: Handle load of the game.
    warn("Should load game from " + file);
  }

  void
  Model::save(const std::string& file) const {
    /// TODO: Handle load of the game.
    warn("Should save game to " + file);
  }

  void
  Model::reset() {
    /// TODO: Handle the reset of the game.
    warn("Should reset game to starting state");
  }

  void
  Model::simulate(const time::Manager& manager) {
    warn("Executing step after " + std::to_string(manager.lastStepDuration()) + " second(s)");
  }

}

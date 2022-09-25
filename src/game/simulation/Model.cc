
# include "Model.hh"

namespace eqdif {

  Model::Model():
    utils::CoreObject("model"),
    Process()
  {
    setService("eqdif");
  }

  void
  Model::simulate(const time::Manager& manager) {
    warn("Executing step after " + std::to_string(manager.lastStepDuration()) + " second(s)");
  }

}

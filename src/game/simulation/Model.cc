
# include "Model.hh"

namespace {

  std::vector<float>
  eulerMethod(const eqdif::SimulationData& /*data*/) {
    return {};
  }

  std::vector<float>
  rungeKutta4(const eqdif::SimulationData& /*data*/) {
    return {};
  }

}

namespace eqdif {

  std::string
  toString(const SimulationMethod& method) noexcept {
    switch (method) {
      case SimulationMethod::EULER:
        return "euler";
      case SimulationMethod::RUNGE_KUTTA_4:
        return "runge-kutta-4";
      default:
        return "unknown";
    }
  }

  std::vector<float>
  computeNextStep(const SimulationData& data) {
    switch (data.method) {
      case SimulationMethod::EULER:
        return eulerMethod(data);
      case SimulationMethod::RUNGE_KUTTA_4:
        return rungeKutta4(data);
      default:
        return {};
    }
  }

}

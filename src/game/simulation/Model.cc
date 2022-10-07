
# include "Model.hh"

# include <iostream>

namespace {

  std::vector<float>
  eulerMethod(const eqdif::SimulationData& data) {
    // https://en.wikipedia.org/wiki/Euler_method
    std::vector<float> out;

    for (unsigned id = 0u ; id < data.vals.size() ; ++id) {
      float sum = 0.0f;

      const std::vector<float> coeffs = data.coeffs[id];
      for (unsigned val = 0u ; val < data.vals.size() ; ++val) {
        sum += data.vals[val] * coeffs[val];
      }

      std::cout <<  "Value " << id << " moved from "
                << data.vals[id] << " to "
                << (data.vals[id] + data.tDelta * sum)
                << std::endl;

      out.push_back(data.vals[id] + data.tDelta * sum);
    }

    return out;
  }

  std::vector<float>
  rungeKutta4(const eqdif::SimulationData& /*data*/) {
    // https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods
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

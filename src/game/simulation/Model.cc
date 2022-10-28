
# include "Model.hh"

# include <iostream>

namespace {

  using LoggerFunc = std::function<void(const std::string&)>;

  std::vector<float>
  eulerMethod(const eqdif::SimulationData& data, LoggerFunc log) {
    // https://en.wikipedia.org/wiki/Euler_method
    std::vector<float> out;

    // Compute derivative from the coefficients.
    std::vector<float> derivatives;

    for (unsigned id = 0u ; id < data.vals.size() ; ++id) {
      float sum = 0.0f;

      const eqdif::Equation& equation = data.system[id];
      for (unsigned coeffId = 0u ; coeffId < equation.size() ; ++coeffId) {
        const eqdif::SingleCoefficient& sf = equation[coeffId];

        float coeff = sf.value;

        for (unsigned val = 0u ; val < sf.dependencies.size() ; ++val) {
          coeff *= data.vals[sf.dependencies[val]];
        }

        sum += coeff;
      }

      derivatives.push_back(sum);
    }

    // Compute new values from the derivatives.
    for (unsigned id = 0u ; id < data.vals.size() ; ++id) {
      log(
        "Value " + std::to_string(id) +  " moved from "
        + std::to_string(data.vals[id]) + " to "
        + std::to_string(data.vals[id] + derivatives[id] * data.tDelta)
      );

      out.push_back(data.vals[id] + derivatives[id] * data.tDelta);
    }

    return out;
  }

  std::vector<float>
  rungeKutta4(const eqdif::SimulationData& /*data*/, LoggerFunc /*log*/) {
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

  Model::Model(const SimulationData& data):
    utils::CoreObject("model"),

    m_data(data)
  {
    setService("eqdif");
  }

  std::vector<float>
  Model::computeNextStep() const {
    const auto logger = [this](const std::string& message) {
      log(message, utils::Level::Verbose);
    };

    switch (m_data.method) {
      case SimulationMethod::EULER:
        return eulerMethod(m_data, logger);
      case SimulationMethod::RUNGE_KUTTA_4:
        return rungeKutta4(m_data, logger);
      default:
        return {};
    }
  }

}

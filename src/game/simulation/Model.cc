
# include "Model.hh"

# include <iostream>
# include <cmath>

namespace {

  float
  computeDerivative(const eqdif::Equation& eq, const std::vector<float>& values) {
    // Compute derivative from the coefficients.
    float derivative = 0.0f;

    for (unsigned coeffId = 0u ; coeffId < eq.coeffs.size() ; ++coeffId) {
      const eqdif::SingleCoefficient& sf = eq.coeffs[coeffId];

      auto coeff = sf.value;

      for (unsigned val = 0u ; val < sf.dependencies.size() ; ++val) {
        const eqdif::VariableDependency& vd = sf.dependencies[val];
        coeff *= std::pow(values[vd.id], vd.n);
      }

      derivative += coeff;
    }

    return derivative;
  }

  float
  eulerMethod(const unsigned id, const std::vector<float>& values, const eqdif::Equation& eq, const float dt) {
    // https://en.wikipedia.org/wiki/Euler_method
    return values[id] + computeDerivative(eq, values) * dt;
  }

  float
  rungeKutta4(const unsigned id, const std::vector<float>& values, const eqdif::Equation& eq, const float dt) {
    // https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods
    // https://www.geeksforgeeks.org/runge-kutta-4th-order-method-solve-differential-equation/
    std::vector<float> tmpValues = values;
    auto originalValue = values[id];

    const float k1 = dt * computeDerivative(eq, tmpValues);

    tmpValues[id] = originalValue + 0.5f * k1;
    const float k2 = dt * computeDerivative(eq, tmpValues);

    tmpValues[id] = originalValue + 0.5f * k2;
    const float k3 = dt * computeDerivative(eq, tmpValues);

    tmpValues[id] = originalValue + 0.5f * k3;
    const float k4 = dt * computeDerivative(eq, tmpValues);

    return values[id] + (1.0f / 6.0f) * (k1 + 2.0f * k2 + 2.0f * k3 + k4);
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

    switch (m_data.method) {
      case SimulationMethod::EULER:
        m_evolve = eulerMethod;
        break;
      case SimulationMethod::RUNGE_KUTTA_4:
        m_evolve = rungeKutta4;
        break;
      default:
        error(
          "Unableo to interpret ",
          "Unknown simulation method " + toString(m_data.method)
        );
        break;
    }
  }

  std::vector<float>
  Model::computeNextStep() const {
    std::vector<float> out(m_data.vals.size(), 0.0f);

    // Compute new values from the derivatives.
    for (unsigned id = 0u ; id < m_data.vals.size() ; ++id) {
      float newValue = m_evolve(
        id,
        m_data.vals,
        m_data.system[id],
        m_data.tDelta
      );

      const auto [lb, hb] = m_data.ranges[id];
      newValue = std::clamp(newValue, lb, hb);

      log(
        m_data.names[id] + " moved from "
        + std::to_string(m_data.vals[id]) + " to "
        + std::to_string(newValue) +
        " (estimate derivative: " + std::to_string((newValue - m_data.vals[id]) / m_data.tDelta) + ")"
      );

      out[id] = newValue;
    }

    return out;
  }

}

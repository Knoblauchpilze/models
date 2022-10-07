
# include "Simulation.hh"
# include <fstream>

namespace eqdif {

  namespace {

    unsigned
    eatEndOfLine(std::ifstream& in) {
      std::string dummy;
      std::getline(in, dummy);

      return dummy.size();
    }

  }

  Simulation::Simulation(const SimulationMethod& method):
    utils::CoreObject("simulation"),
    Process(),

    m_method(method)
  {
    setService("eqdif");
    addModule(toString(m_method));

    m_variableNames.push_back("haha");
    m_initialValues.push_back(0.2f);

    m_variableNames.push_back("hihi");
    m_initialValues.push_back(-1.9f);

    m_coefficients.push_back({1.0f, 0.0f});
    m_coefficients.push_back({1.0f, -1.0f});

    m_values.push_back({0.2f, 0.5f});
    m_values.push_back({0.4f, 0.6f});
  }

  void
  Simulation::load(const std::string& file) {
    // Open the file and verify that it is valid.
    std::ifstream in(file.c_str());
    if (!in.good()) {
      error(
        "Failed to load model to \"" + file + "\"",
        "Failed to open file"
      );
    }

    // Read the number of variables.
    unsigned count = 0u;
    in >> count;

    m_variableNames.clear();
    m_initialValues.clear();

    // Read all variables.
    for (unsigned id = 0u ; id < count ; ++id) {
      std::string name;
      float initialValue = 0.0f;

      in >> name;
      in >> initialValue;

      log("Loaded variable " + name + " with initial value " + std::to_string(initialValue));

      m_variableNames.push_back(name);
      m_initialValues.push_back(initialValue);

      eatEndOfLine(in);

      // Read the coefficients.
      std::vector<float> coeffs;

      for (unsigned val = 0u ; val < count ; ++val) {
        float coeff = 0.0f;
        in.read(reinterpret_cast<char*>(&coeff), sizeof(float));
        coeffs.push_back(coeff);
      }

      // Read the rest of the line.
      if (auto discarded = eatEndOfLine(in); discarded > 0) {
        // The division comes from the fact that in each step we expect
        // floating point values for coefficients. The remaining characters
        // are probably unknown coefficients.
        warn(
          "Discarded " + std::to_string(discarded / sizeof(float)) +
          " character(s) (" + std::to_string(discarded) +
          " byte(s)) for coefficient(s) of " + name
        );
      }

      m_coefficients.push_back(coeffs);
      log("Read " + std::to_string(coeffs.size()) + " coefficient(s) for variable " + name);
    }

    // Read simulation steps.
    in >> count;

    log("Will read " + std::to_string(count) + " step(s)");
    eatEndOfLine(in);

    m_values.clear();

    for (unsigned id = 0u ; id < count ; ++id) {
      std::vector<float> step;

      for (unsigned val = 0u ; val < m_variableNames.size() ; ++val) {
        float value = 0.0f;
        in.read(reinterpret_cast<char*>(&value), sizeof(float));
        step.push_back(value);
      }

      // Read the rest of the line.
      if (auto discarded = eatEndOfLine(in); discarded > 0) {
        // The division comes from the fact that in each step we expect
        // floating point values for variables. The remaining characters
        // are probably unknown variables.
        warn(
          "Discarded " + std::to_string(discarded / sizeof(float)) +
          " character(s) (" + std::to_string(discarded) +
          " byte(s)) for step " + std::to_string(id)
        );
      }

      m_values.push_back(step);
      log("Read " + std::to_string(step.size()) + " value(s) for step " + std::to_string(id));
    }

    info(
      "Loaded simulation with " + std::to_string(m_variableNames.size()) +
      " variable(s) and " + std::to_string(m_values.size()) +
      " simulation step(s) from " + file
    );
  }

  void
  Simulation::save(const std::string& file) const {
    // Open the file and verify that it is valid.
    std::ofstream out(file.c_str());
    if (!out.good()) {
      error(
        "Failed to save model to \"" + file + "\"",
        "Failed to open file"
      );
    }

    // Save the number of variables.
    out << m_variableNames.size() << std::endl;

    float buf, size = sizeof(float);
    const char* raw = reinterpret_cast<const char*>(&buf);

    // Save the name of each variable along its initial value.
    for (unsigned id = 0u ; id < m_variableNames.size() ; ++id) {
      out << m_variableNames[id] << std::endl;
      out << m_initialValues[id] << std::endl;

      // Save the coefficients.
      const std::vector<float>& coeffs = m_coefficients[id];

      for (unsigned val = 0u ; val < coeffs.size() ; ++val) {
        buf = coeffs[val];
        out.write(raw, size);
      }

      out << std::endl;
    }

    // Save the number of simulation values.
    out << m_values.size() << std::endl;

    // And then each simulation values.
    for (unsigned id = 0u ; id < m_values.size() ; ++id) {
      const std::vector<float>& step = m_values[id];

      for (unsigned val = 0u ; val < step.size() ; ++val) {
        buf = step[val];
        out.write(raw, size);
      }

      out << std::endl;
    }

    info(
      "Saving simulation with " + std::to_string(m_variableNames.size()) +
      " variable(s) and " + std::to_string(m_values.size()) +
      " simulation step(s) to " + file
    );
  }

  void
  Simulation::reset() {
    info(
      "Reset " + std::to_string(m_variableNames.size()) +
      " variable(s) to their initial value, discarding " +
      std::to_string(m_values.size()) + " existing simulation step(s)"
    );

    m_values = {m_initialValues};
  }

  void
  Simulation::simulate(const time::Manager& manager) {
    // https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods
    SimulationData data{
      m_initialValues,           // vals0
      m_coefficients,            // coeffs

      m_values.back(),           // vals

      m_method,                  // method
      manager.lastStepDuration() // tDelta
    };

    auto nextStep = computeNextStep(data);
    if (nextStep.size() != m_variableNames.size()) {
      error(
        "Failed to generate value for all " + std::to_string(m_variableNames.size()) +
        " variable(s) for step " + std::to_string(m_values.size()),
        "Only " + std::to_string(nextStep.size()) +
        " value(s) were generated"
      );
    }

    log(
      "Generated " + std::to_string(nextStep.size()) +
      " value(s) for step step " + std::to_string(m_values.size()) +
      " lasting " + std::to_string(manager.lastStepDuration(time::Unit::Millisecond)) +
      "ms"
    );

    m_values.push_back(nextStep);
  }

}
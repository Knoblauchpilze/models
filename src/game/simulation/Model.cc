
# include "Model.hh"
# include <fstream>

namespace eqdif {

  Model::Model():
    utils::CoreObject("model"),
    Process()
  {
    setService("eqdif");

    m_variableNames.push_back("haha");
    m_initialValues.push_back(0.2f);

    m_values.push_back({0.2f, 0.5f});
    m_values.push_back({0.4f, 0.6f});
  }

  void
  Model::load(const std::string& file) {
    /// TODO: Handle load of the game.
    warn("Should load game from " + file);
  }

  void
  Model::save(const std::string& file) const {
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

    // Save the name of each variable along its initial value.
    for (unsigned id = 0u ; id < m_variableNames.size() ; ++id) {
      out << m_variableNames[id] << std::endl;
      out << m_initialValues[id] << std::endl;
    }

    // Save the number of simulation values.
    out << m_values.size() << std::endl;

    // And then each simulation values.
    float buf, size = sizeof(float);
    const char* raw = reinterpret_cast<const char*>(&buf);

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
  Model::reset() {
    /// TODO: Handle the reset of the game.
    info(
      "Reset " + std::to_string(m_variableNames.size()) +
      " variable(s) to their initial value, discarding " +
      std::to_string(m_values.size()) + " existing simulation step(s)"
    );

    m_values = {m_initialValues};
  }

  void
  Model::simulate(const time::Manager& manager) {
    warn("Executing step after " + std::to_string(manager.lastStepDuration()) + " second(s)");
  }

}

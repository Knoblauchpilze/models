
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

    m_method(method),

    onSimulationStep()
  {
    setService("eqdif");
    addModule(toString(m_method));

    initialize();

    validate();
  }

  Simulation::~Simulation() {
    onSimulationStep.disconnectAll();
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
    m_system.clear();

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

      // Read the equation for this variable.
      unsigned coefficientsCount = 0u;
      in.read(reinterpret_cast<char*>(&coefficientsCount), sizeof(unsigned));

      Equation eq;

      for (unsigned coeff = 0u ; coeff < coefficientsCount ; ++coeff) {
        SingleCoefficient sf;

        // Read the coefficient's value.
        in.read(reinterpret_cast<char*>(&sf.value), sizeof(float));

        // And the dependencies.
        unsigned depCount = 0u;
        in.read(reinterpret_cast<char*>(&depCount), sizeof(unsigned));

        for (unsigned sfId = 0u ; sfId < depCount ; ++sfId) {
          VariableDependency vd{0u, 1.0f};

          in.read(reinterpret_cast<char*>(&vd.id), sizeof(unsigned));
          in.read(reinterpret_cast<char*>(&vd.n), sizeof(float));

          sf.dependencies.push_back(vd);
        }

        eq.push_back(sf);
      }

      // Read the rest of the line.
      if (auto discarded = eatEndOfLine(in); discarded > 0) {
        // The division comes from the fact that in each step we expect
        // floating point values for coefficients. The remaining characters
        // are probably unknown coefficients.
        warn(
          "Discarded " + std::to_string(discarded) +
          " byte(s)) for equation for " + name
        );
      }

      m_system.push_back(eq);
      log(
        "Read equation with " + std::to_string(eq.size()) +
        " coefficient(s) for variable " + name
      );
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
    }

    info(
      "Loaded simulation with " + std::to_string(m_variableNames.size()) +
      " variable(s) and " + std::to_string(m_values.size()) +
      " simulation step(s) from " + file
    );

    validate();
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

    float bufF, sizeF = sizeof(float);
    const char* rawF = reinterpret_cast<const char*>(&bufF);

    unsigned bufU, sizeU = sizeof(unsigned);
    const char* rawU = reinterpret_cast<const char*>(&bufU);

    // Save the name of each variable along its initial value.
    for (unsigned id = 0u ; id < m_variableNames.size() ; ++id) {
      out << m_variableNames[id] << std::endl;
      out << m_initialValues[id] << std::endl;

      // Save the equation for this variable.
      const Equation& eq = m_system[id];

      bufU = eq.size();
      out.write(rawU, sizeU);

      for (unsigned coeff = 0u ; coeff < eq.size() ; ++coeff) {
        // Save the coefficients.
        const SingleCoefficient& sf = eq[coeff];

        bufF = sf.value;
        out.write(rawF, sizeF);

        bufU = sf.dependencies.size();
        out.write(rawU, sizeU);

        for (unsigned sfId = 0u ; sfId < sf.dependencies.size() ; ++sfId) {
          bufU = sf.dependencies[sfId].id;
          out.write(rawU, sizeU);

          bufF = sf.dependencies[sfId].n;
          out.write(rawF, sizeF);
        }
      }

      out << std::endl;
    }

    // Save the number of simulation values.
    out << m_values.size() << std::endl;

    // And then each simulation values.
    for (unsigned id = 0u ; id < m_values.size() ; ++id) {
      const std::vector<float>& step = m_values[id];

      for (unsigned val = 0u ; val < step.size() ; ++val) {
        bufF = step[val];
        out.write(rawF, sizeF);
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

    validate();
  }

  void
  Simulation::simulate(const time::Manager& manager) {
    SimulationData data{
      m_system,                  // system

      m_variableNames,           // names

      m_values.back(),           // vals

      m_method,                  // method
      manager.lastStepDuration() // tDelta
    };

    Model model(data);
    auto nextStep = model.computeNextStep();
    if (nextStep.size() != m_variableNames.size()) {
      error(
        "Failed to generate values for all " + std::to_string(m_variableNames.size()) +
        " variable(s) for step " + std::to_string(m_values.size()),
        "Only " + std::to_string(nextStep.size()) +
        " value(s) were generated"
      );
    }

    log(
      "Generated " + std::to_string(nextStep.size()) +
      " value(s) for step step " + std::to_string(m_values.size()) +
      " lasting " + std::to_string(manager.lastStepDuration(time::Unit::Millisecond)) +
      "ms",
      utils::Level::Verbose
    );

    m_values.push_back(nextStep);

    onSimulationStep.emit(nextStep);
  }

  const std::vector<std::string>&
  Simulation::getVariableNames() const noexcept {
    return m_variableNames;
  }

  void
  Simulation::initialize() {
// # define DUMMY_SIMULATION
// # define PREY_PREDATOR_SIMULATION
# ifdef DUMMY_SIMULATION
    unsigned count = 10u;

    for (unsigned id = 0u ; id < count ; ++id) {
      m_variableNames.push_back("haha_" + std::to_string(id));
      m_initialValues.push_back(0.2f * (id + 1.0f));

      Equation eq{{1.0f, {{id, 1.0f}}}};

      for (unsigned pad = 1u ; pad < count ; ++pad) {
        eq.push_back({0.0f, {}});
      }

      m_system.push_back(eq);
    }
# elif defined(PREY_PREDATOR_SIMULATION)
    // See here: https://en.wikipedia.org/wiki/Lotka%E2%80%93Volterra_equations
    constexpr auto preyCount = 15.0f;
    constexpr auto alpha = 0.9f;
    constexpr auto beta = 0.2f;

    m_variableNames.push_back("prey");
    m_initialValues.push_back(preyCount);

    Equation eqPrey{
      {alpha, {{0u, 1.0f}}},
      {-beta, {{0u, 1.0f}, {1u, 1.0f}}}
    };
    m_system.push_back(eqPrey);

    // Predators.
    constexpr auto predCount = 1.0f;
    constexpr auto delta = 0.2f;
    constexpr auto gamma = 0.5f;

    m_variableNames.push_back("predator");
    m_initialValues.push_back(predCount);

    Equation eqPred{
      {delta, {{0u, 1.0f}, {1u, 1.0f}}},
      {-gamma, {{1u, 1.0f}}}
    };

    m_system.push_back(eqPred);
# else
    const auto pad = [](int count, eqdif::Equation& eq) {
      if (count <= 0) {
        return;
      }

      eq.insert(eq.end(), count, {0.0f, {}});
    };

    constexpr auto FOOD = 0u;
    constexpr auto POP = FOOD + 1u;
    constexpr auto INDUSTRIAL_PROD = POP + 1u;
    constexpr auto POLLUTION = INDUSTRIAL_PROD + 1u;
    constexpr auto COUNT = POLLUTION + 1u;

    // Food.
    m_variableNames.push_back("food");
    m_initialValues.push_back(10.0f);

    constexpr auto CROP_YIELD = 0.02f;
    constexpr auto APPETITE = -0.1f;

    m_system.push_back({
      {CROP_YIELD, {{FOOD, 2.0f}}},
      {APPETITE, {{POP, 1.0f}}}
    });
    pad(COUNT - m_system.back().size(), m_system.back());

    // Population.
    m_variableNames.push_back("pop");
    m_initialValues.push_back(100.0f);

    constexpr auto MORTALITY_RATE = -0.01f;
    constexpr auto BIRTH_RATE = 0.015f;
    constexpr auto POLLUTION_MORTALITY = -0.2f;
    m_system.push_back({
      {MORTALITY_RATE, {{POP, 1.0f}}},
      {BIRTH_RATE, {{POP, 1.0f}, {FOOD, 1.0f}}},
      {POLLUTION_MORTALITY, {{POLLUTION, 2.0f}}}
    });
    pad(COUNT - m_system.back().size(), m_system.back());

    // Industrial production.
    m_variableNames.push_back("industrial");
    m_initialValues.push_back(0.0f);

    constexpr auto PRODUCTIVITY = 0.01f;
    constexpr auto INDUSTRY_DEPRECATION = -0.001f;

    m_system.push_back({
      {PRODUCTIVITY, {{POP, 1.0f}}},
      {INDUSTRY_DEPRECATION, {{INDUSTRIAL_PROD, 1.0f}}}
    });
    pad(COUNT - m_system.back().size(), m_system.back());

    // Pollution.
    m_variableNames.push_back("pollution");
    m_initialValues.push_back(0.0f);

    constexpr auto POLLUTION_RATE = 0.1f;
    constexpr auto PURGE_RATE = -0.05f;

    m_system.push_back({
      {POLLUTION_RATE, {{INDUSTRIAL_PROD, 2.0f}}},
      {PURGE_RATE, {{POLLUTION, 2.0f}}}
    });
    pad(COUNT - m_system.back().size(), m_system.back());
# endif

    m_values.push_back(m_initialValues);
  }

  void
  Simulation::validate() {
    auto varsCount = m_variableNames.size();
    auto varsInitValues = m_initialValues.size();

    if (varsCount != varsInitValues) {
      error(
        "Mismatch between defined variables and values",
        "Found " + std::to_string(varsCount) + " variable(s) but " +
        std::to_string(varsInitValues) + " value(s)"
      );
    }

    auto relationsCount = m_system.size();

    if (varsCount != relationsCount) {
      error(
        "Mismatch between defined variable(s) and equation(s)",
        "Found " + std::to_string(varsCount) + " variable(s) but " +
        std::to_string(relationsCount) + " equation(s)"
      );
    }

    for (unsigned id = 0u ; id < m_system.size() ; ++id) {
      auto relationsForVariable = m_system[id].size();

      if (varsCount != relationsForVariable) {
        error(
          "Mismatch between defined variables and coefficients",
          "Variable " + m_variableNames[id] + " defines " +
          std::to_string(relationsForVariable) + " coefficient(s) but " +
          std::to_string(varsCount) + " variable(s) are defined"
        );
      }
    }

    for (unsigned eqId = 0u ; eqId < m_system.size() ; ++eqId) {
      const Equation& eq = m_system[eqId];

      for (unsigned sf = 0u ; sf < eq.size() ; ++sf) {
        const SingleCoefficient& coeff = eq[sf];

        for (unsigned dep = 0u ; dep < coeff.dependencies.size() ; ++dep) {
          if (coeff.dependencies[dep].id > m_variableNames.size()) {
            error(
              "Dependency for variable " + m_variableNames[eqId] + " requires " +
              std::to_string(coeff.dependencies[dep].id) + " variable(s) when only " +
              std::to_string(varsCount) + " are available"
            );
          }
        }
      }
    }

    for (unsigned id = 0u ; id < m_values.size() ; ++id) {
      auto valuesForStep = m_values[id].size();

      if (varsCount != valuesForStep) {
        error(
          "Mismatch between defined variables and steps",
          "Step " + std::to_string(id) + " defines " +
          std::to_string(valuesForStep) + " value(s) but " +
          std::to_string(varsCount) + " variable(s) are defined"
        );
      }
    }
  }

}

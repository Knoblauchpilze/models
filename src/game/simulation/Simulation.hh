#ifndef    SIMULATION_HH
# define   SIMULATION_HH

# include <vector>
# include <core_utils/CoreObject.hh>
# include <core_utils/Signal.hh>
# include "Launcher.hh"
# include "Model.hh"

namespace eqdif {

  class Simulation: public utils::CoreObject, public Process {
    public:

      Simulation(const SimulationMethod& method);

      virtual ~Simulation();

      void
      load(const std::string& file);

      void
      save(const std::string& file) const;

      void
      reset();

      void
      simulate(const time::Manager& manager) override;

      const std::vector<std::string>&
      getVariableNames() const noexcept;

    private:

      /**
       * @brief - Initialize the simulation.
       *
       */
      void
      initialize();

      /**
       * @brief - Used to verify that the simulation respects
       *          some properties and is in general consistent.
       */
      void
      validate();

    private:

      /// @brief - The simulation method: used to determine how
      /// to compute the next step of the variables.
      SimulationMethod m_method;

      /// @brief - The list of variables and their names.
      std::vector<std::string> m_variableNames;

      /// @brief - The initial values for the variables.
      std::vector<float> m_initialValues;

      /// @brief - The bounds for the variables.
      std::vector<Range> m_ranges;

      /// @brief - The linear combination of each variable
      /// on each of the other variables.
      System m_system;

      /// @brief - The values of the variables for each
      /// timestamp.
      std::vector<std::vector<float>> m_values;

    public:

      /**
       * @brief - Signal which notifies that a new simulation step
       *          has been computed.
      */
      utils::Signal<const std::vector<float>&> onSimulationStep;
  };

}

#endif    /* SIMULATION_HH */

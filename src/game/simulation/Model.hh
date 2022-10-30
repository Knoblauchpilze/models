#ifndef    MODEL_HH
# define   MODEL_HH

# include <string>
# include <vector>
# include <core_utils/CoreObject.hh>

namespace eqdif {

  /// @brief - The computation method to evolve the data.
  enum class SimulationMethod {
    EULER,
    RUNGE_KUTTA_4
  };

  /**
   * @brief - Convert the simulation mode to a readable string.
   * @param - the mode to convert.
   * @return - the name of the simulation method.
   */
  std::string
  toString(const SimulationMethod& method) noexcept;

  /// @brief - In general an equation can look something like this:
  /// dx = Ax - Bxy
  /// dy = Cxy - Dy
  /// To represent that in a generic way, we need a way to represent
  /// the dependencies for a single coefficient (this is the `Bxy`).
  struct SingleCoefficient {
    float value;
    std::vector<unsigned> dependencies;
  };

  /// Then the list of coefficients for a single variable.
  using Equation = std::vector<SingleCoefficient>;
  /// And finally the list of coefficients for each variable.
  using System = std::vector<Equation>;

  /// @brief - Convenience data storing all the needed info
  /// on the simulation to evolve.
  struct SimulationData {
    /// @brief - The linear dependencies of variables on one
    /// another.
    const System& system;

    /// @brief - The current value of the variables.
    const std::vector<float>& vals;

    /// @brief - The simulation method to use to compute the
    /// next step of the values.
    SimulationMethod method;

    /// @brief - The simulation time step: describes how far
    /// in the future the values should be predicted.
    float tDelta;
  };

  /// @brief - An interface for the evolution method.
  using EvolutionMethod = std::function<float(const unsigned, const std::vector<float>&, const Equation&, const float)>;

  class Model: public utils::CoreObject {
    public:

      Model(const SimulationData& data);

      std::vector<float>
      computeNextStep() const;

    private:

      const SimulationData& m_data;

      /// @brief - The evolution method: this is computed from the
      /// simulation method attached to this model.
      EvolutionMethod m_evolve;
  };

}

#endif    /* MODEL_HH */

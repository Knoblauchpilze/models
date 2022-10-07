#ifndef    MODEL_HH
# define   MODEL_HH

# include <string>
# include <vector>

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

  /// @brief - Convenience data storing all the needed info
  /// on the simulation to evolve.
  struct SimulationData {
    /// @brief - The initial values.
    std::vector<float> vals0;

    /// @brief - The linear dependencies of variables on one
    /// another.
    std::vector<std::vector<float>> coeffs;

    /// @brief - The current value of the variables.
    std::vector<float> vals;

    /// @brief - The simulation method to use to compute the
    /// next step of the values.
    SimulationMethod method;

    /// @brief - The simulation time step: describes how far
    /// in the future the values should be predicted.
    float tDelta;
  };

  std::vector<float>
  computeNextStep(const SimulationData& data);

}

#endif    /* MODEL_HH */

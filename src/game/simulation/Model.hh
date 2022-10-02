#ifndef    MODEL_HH
# define   MODEL_HH

# include <deque>
# include <core_utils/CoreObject.hh>
# include "Launcher.hh"

namespace eqdif {

  class Model: public utils::CoreObject, public Process {
    public:

      Model();

      void
      load(const std::string& file);

      void
      save(const std::string& file) const;

      void
      reset();

      void
      simulate(const time::Manager& manager) override;

    private:

      /// @brief - The list of variables and their names.
      std::vector<std::string> m_variableNames;

      /// @brief - The initial values for the variables.
      std::vector<float> m_initialValues;

      /// @brief - The values of the variables for each
      /// timestamp.
      std::vector<std::vector<float>> m_values;
  };

}

#endif    /* MODEL_HH */

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

  };

}

#endif    /* MODEL_HH */

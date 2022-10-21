#ifndef    EQUATION_VIEW_HH
# define   EQUATION_VIEW_HH

# include <memory>
# include <core_utils/CoreObject.hh>
# include "olcEngine.hh"
# include "Menu.hh"

namespace pge {

  class EquationView: public utils::CoreObject {
    public:

      EquationView(const olc::vi2d& pos,
                   const olc::vi2d& size,
                   const std::string& name);

      ~EquationView();

      /**
       * @brief- Interface method allowing to render a menu in
       *         a parent application. This is used to offload
       *         some of the rendering code from the main app
       *         and hide the internal complexity of the menu.
       *         Note: we draw on the active layer so it has
       *         to be configured before calling this method.
       * @param pge - the rendering engine to display the menu.
       */
      void
      render(olc::PixelGameEngine* pge) const;

      /**
       * @brief - Used to process the user input defined in
       *          the argument and update the internal state
       *          of this menu if needed.
       * @param c - the controls and user input for this
       *            frame.
       * @param actions - the list of actions produced by the
       *                  menu while processing the events.
       * @return - the description of what happened when the
       *           inputs has been processed.
       */
      menu::InputHandle
      processUserInput(const controls::State& c,
                       std::vector<ActionShPtr>& actions);

    private:

      olc::vf2d m_pos;
      olc::vi2d m_size;

      olc::Pixel m_color;
  };

  using EquationViewShPtr = std::shared_ptr<EquationView>;
}

#endif    /* EQUATION_VIEW_HH */

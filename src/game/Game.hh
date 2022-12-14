#ifndef    GAME_HH
# define   GAME_HH

# include <vector>
# include <memory>
# include <core_utils/CoreObject.hh>
# include <core_utils/TimeUtils.hh>
# include "Launcher.hh"
# include "Simulation.hh"

namespace pge {

  // Forward declaration of the `Menu` class to be able
  // to use it right away.
  class Menu;
  using MenuShPtr = std::shared_ptr<Menu>;

  class Game: public utils::CoreObject {
    public:

      /**
       * @brief - Create a new game with default parameters.
       */
      Game();

      ~Game();

      /**
       * @brief - Used to perform the creation of the menus
       *          allowing to control the world wrapped by
       *          this game.
       * @param width - the zidth of the window in pixels into
                        which this menu will be inserted.
       * @param height - the height of the window in pixels
       *                 into which this menu will be inserted.
       * @return - the list of menus representing this game.
       */
      virtual std::vector<MenuShPtr>
      generateMenus(float width,
                    float height);

      /**
       * @brief - Used to create a tower with the specified
       *          type (as defined by the `setTowerType`)
       *          method at the specified position. Note
       *          that we will only perform the action if
       *          no elements exist here.
       *          Also note that the coordinates are used as
       *          is and should thus correspond to values that
       *          interpretable by the underlying game data.
       * @param x - the abscissa of the position at which the
       *            action should be taking place.
       * @param y - the ordinate of the position at which the
       *            action will be taking place.
       */
      void
      performAction(float x, float y);

      /**
       * @brief - Requests the game to be terminated. This is
       *          applied to the next iteration of the game
       *          loop.
       */
      void
      terminate() noexcept;

      /**
       * @brief - Returns whether or not the game has been
       *          terminated. The game is terminated when
       *          the user wants to exit the app (usually).
       */
      bool
      terminated() const noexcept;

      /**
       * @brief - Forward the call to step one step ahead
       *          in time to the internal world.
       * @param tDelta - the duration of the last frame in
       *                 seconds.
       * @param bool - `true` in case the game continues,
       *               and `false` otherwise (i.e. if the
       *               game is ended).
       */
      bool
      step(float tDelta);

      /**
       * @brief - Performs the needed operation to handle
       *          the pause and resume operation for this
       *          game. It will automatically disable the
       *          menu if needed or make it visible again.
       */
      void
      togglePause();

      /**
       * @brief - Used to indicate that the world should be
       *          paused. Time based entities and actions
       *          should take actions to correctly resume at
       *          a later time.
       */
      void
      pause();

      /**
       * @brief - Used to indicate that the world should be
       *          resuming its activity. Time based entities
       *          should take actions to be resuming their
       *          pathes, motions, etc.
       */
      void
      resume();

      /**
       * @brief - Loads the board defined in the input file.
       * @param file - the file to use to load a board from.
       */
      void
      load(const std::string& file);

      /**
       * @brief - Save the current state of the board to a default
       *          file with the name provided in input.
       * @param file - the file to save the board into.
       */
      void
      save(const std::string& file) const;

      void
      speedUpSimulation() noexcept;

      void
      resetSimulation() noexcept;

      void
      simulateNextStep() noexcept;

      void
      toggleSimulationStatus() noexcept;

      eqdif::Simulation&
      getSimulation() noexcept;

      const eqdif::Simulation&
      getSimulation() const noexcept;

    private:

      /**
       * @brief - Used to enable or disable the menus that
       *          compose the game. This allows to easily
       *          hide any game related component.
       * @param enable - `true` if the menus are enabled.
       */
      void
      enable(bool enable);

      /**
       * @brief - Used during the step function and by any process
       *          that needs to update the UI and the text content
       *          of menus.
       */
      virtual void
      updateUI();

    private:

      /// @brief - Convenience structure allowing to group information
      /// about a timed menu.
      struct TimedMenu {
        // Information about when the menu started appearing.
        utils::TimeStamp date;

        // Keep track of whether the menu was already active.
        bool wasActive;

        // The alert menu indicating controlled by this object.
        MenuShPtr menu;

        // The duration of the alert.
        int duration;

        /**
         * @brief - Used to update the internal attribute with
         *          the current value of whether the menu should
         *          be active or not.
         * @param active - `true` if the menu should still be
         *                 active.
         * @return - `true` if the menu is still visible.
         */
        bool
        update(bool active) noexcept;
      };

      /// @brief - Convenience information defining the state of the
      /// game. It includes information about whether the menus should
      /// be displayed and if the user actions should be interpreted
      /// or not.
      struct State {
        // Defines whether this world is paused (i.e.
        // internal attributes of the mobs/blocks/etc
        // have already been updated to reflect the
        // pause status) or not. This allows to react
        // to consecutive pause requests and prevent
        // weird behaviors to occur.
        bool paused;

        // Whether or not the UI is disabled.
        bool disabled;

        // Used to hold whether or not the game has been shut
        // down. It usually indicates that no simulation will
        // be performed anymore and usually indicates that a
        // termination request has been received.
        bool terminated;

        // The current speed of the simulation.
        float speed;

        // Whether or not the simulation was running before
        // a pause event was triggered.
        bool wasRunning;

        // Whether or not a reset event was triggered.
        bool resetTriggered;
      };

      /// @brief - Convenience structure allowing to regroup
      /// all info about the menu in a single struct.
      struct Menus {
        MenuShPtr reset;

        MenuShPtr speed;

        MenuShPtr timestamp;

        MenuShPtr nextStep;

        MenuShPtr startPause;

        TimedMenu resetAlert;
      };

      /**
       * @brief - The definition of the game state.
       */
      State m_state;

      /**
       * @brief - The menus displaying information about the
       *          current state of the simulation.
       */
      Menus m_menus;

      /**
       * @brief - The model to simulate.
       */
      eqdif::Simulation m_simulation;

      /**
       * @brief - The launcher used to simuate the environment.
       */
      eqdif::Launcher m_launcher;

    public:

      /**
       * @brief - Signal which notifies that a the simulation has
       *          been reset.
      */
      utils::Signal<> onSimulationReset;
  };

  using GameShPtr = std::shared_ptr<Game>;
}

# include "Game.hxx"

#endif    /* GAME_HH */

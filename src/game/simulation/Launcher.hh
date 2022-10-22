#ifndef    LAUNCHER_HH
# define   LAUNCHER_HH

# include <mutex>
# include <memory>
# include <thread>
# include <core_utils/CoreObject.hh>
# include "Manager.hh"

namespace eqdif {

  /// @brief - Enumeration defining the state of the simulation.
  enum class State {
    None,
    RunRequested,
    Running,
    PauseRequested,
    Paused,
    ResumeRequested,
    StopRequested,
    Stopped
  };

  /// @brief - The description of an object which can be simulated
  /// but this launcher.
  class Process {
    public:

      /**
       * @brief - Compute the next step of the simulation.
       * @param manager - information about the simulation's time.
       */
      virtual void
      simulate(const time::Manager& manager) = 0;
  };

  /// @brief - An operation which is to be performed when
  /// the simulation is locked.
  using LockedOperation = std::function<void(Process&)>;

  /**
   * @brief - Convert a state to a human readable string.
   * @param state - the state to convert.
   * @return - the corresponding string.
   */
  std::string
  stateToString(const State& state) noexcept;

  class Launcher: public utils::CoreObject {
    public:

      /**
       * @brief - Create a new launcher with the desired properties.
       * @param process - the process to simulate.
       * @param fps - the desired framerate for the simulation.
       * @param step - the duration of each simulation step.
       * @param unit - the unit of the step (used to interpret the
       *               previous parameter).
       */
      Launcher(Process* process,
               float fps,
               float step,
               const time::Unit& unit);

      /**
       * @brief - Release the resource used by this launcher and
       *          handles gracefully shutting down the simulation.
       */
      ~Launcher();

      /**
       * @brief - Return the current desired framerate that this
       *          launcher tries to maintain.
       * @return - the desired framerate.
       */
      float
      desiredFPS() const noexcept;

      /**
       * @brief - Return the current state of the simulation. Note
       *          that it only represent the state at the moment of
       *          calling the method.
       * @return - the current state of the simulation.
       */
      State
      state() const noexcept;

      /**
       * @brief - Define a new value for the desired FPS. Nothing
       *          happens if the framerate is negative or zero.
       * @param fps - the desired framerate.
       */
      void
      setDesiredFramerate(float fps);

      /**
       * @brief - Start the simulation. Nothing happens in case
       *          it is already running.
       */
      void
      start();

      /**
       * @brief - Pause the simulation. Nothing happens in case
       *          the simulation is stopped or already paused.
       */
      void
      pause();

      /**
       * @brief - Resume the simulation. Nothing happens in case
       *          it is already running.
       */
      void
      resume();

      /**
       * @brief - Stop the simulation. Nothing happens in case
       *          it is not started.
       */
      void
      stop();

      /**
       * @brief - Perform a single simulation step. Nothing happens
       *          in case the simulation is running. Otherwise the
       *          state is restored to its previous state.
       */
      void
      step();

      /**
       * @brief - Execute the provided function after locking the
       *          internal locker.
       * @param - the operation to execute on the wrapped process.
       */
      void
      performOperation(LockedOperation op) const;

      /**
       * @brief - Return the amount of time elapsed since the origin
       *          of the simulation in seconds.
       * @return - the number of seconds elapsed.
       */
      float
      elapsed() const noexcept;

    private:

      /**
       * @brief - Asynchronous method launched in a thread which
       *          allows to simulate the environment attached to
       *          the launcher at regular intervals.
       */
      void
      asynchronousRunningLoop();

      /**
       * @brief - Used to run a single simulation step. The input
       *          boolean indicates whether the method should make
       *          the current thread sleep in order to maintain
       *          the desired FPS or not.
       *          Note that we put the desired FPS in parameter to
       *          have a state less function.
       * @param sleep - `true` if the FPS should be considered or
       *                not.
       * @param desiredFPS - the desired framerate to maintain.
       */
      void
      simulate(bool sleep, float desiredFPS);

    private:

      /// @brief - Convenience define for a lock guard.
      using Guard = std::lock_guard<std::mutex>;

      /**
       * @brief - The process attached to this launcher.
       */
      Process* m_process;

      /**
       * @brief - A mutex to protect the access to the simulation
       *          thread.
       */
      mutable std::mutex m_simThreadLocker;

      /**
       * @brief - The thread used to handle the simulation. This
       *          is initialized only when the simulation starts.
       */
      std::shared_ptr<std::thread> m_simThread;

      /**
       * @brief - The state of the simulation. Updated with the
       *          latest status of the execution.
       */
      State m_state;

      /**
       * @brief - The desired framerate for the simulation.
       */
      float m_desiredFPS;

      /**
       * @brief - The duration of a single simulation step.
       */
      float m_step;

      /**
       * @brief - The unit of the simulation step.
       */
      time::Unit m_stepUnit;

      /**
       * @brief - The object allowing to manage the time passing
       *          in the simulation.
       */
      time::Manager m_time;
  };

}

#endif    /* LAUNCHER_HH */

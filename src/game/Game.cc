
# include "Game.hh"
# include <sstream>
# include <cxxabi.h>
# include "Menu.hh"

namespace {

/// @brief - The duration of the alert prompting that
/// the current player is in check, in stalemate or
/// won/lost.
# define ALERT_DURATION_MS 3000

/// @brief - The height of the main menu.
constexpr auto STATUS_MENU_HEIGHT = 50;

/// @brief - The maximum speed for the simulation.
constexpr auto MAX_SIMULATION_SPEED = 32.0f;

constexpr auto RESET_SIMULATION_TEXT = "Reset";
constexpr auto NEXT_STEP_SIMULATION_TEXT = "Next step";

constexpr auto START_SIMULATION_TEXT = "Start";
constexpr auto PAUSE_SIMULATION_TEXT = "Pause";

constexpr auto DESIRED_SIMULATION_FPS = 80.0f;

  pge::MenuShPtr
  generateMenu(const olc::vi2d& pos,
               const olc::vi2d& size,
               const std::string& text,
               const std::string& name,
               const olc::Pixel& color,
               bool clickable = false,
               bool selectable = false)
  {
    pge::menu::MenuContentDesc fd = pge::menu::newMenuContent(text, "", size);
    fd.color = olc::WHITE;
    fd.hColor = olc::GREY;
    fd.align = pge::menu::Alignment::Center;

    return std::make_shared<pge::Menu>(
      pos,
      size,
      name,
      pge::menu::newColoredBackground(color),
      fd,
      pge::menu::Layout::Horizontal,
      clickable,
      selectable
    );
  }

  pge::MenuShPtr
  generateMessageBoxMenu(const olc::vi2d& pos,
                         const olc::vi2d& size,
                         const std::string& text,
                         const std::string& name,
                         bool alert)
  {
    pge::menu::MenuContentDesc fd = pge::menu::newMenuContent(text, "", size);
    fd.color = (alert ? olc::RED : olc::GREEN);
    fd.align = pge::menu::Alignment::Center;

    return std::make_shared<pge::Menu>(
      pos,
      size,
      name,
      pge::menu::newColoredBackground(alert ? olc::VERY_DARK_RED : olc::VERY_DARK_GREEN),
      fd,
      pge::menu::Layout::Horizontal,
      false,
      false
    );
  }

}

namespace pge {

  Game::Game():
    utils::CoreObject("game"),

    m_state(
      State{
        true,  // paused
        true,  // disabled
        false, // terminated
        1.0f,  // speed
        false, // wasRunning
        false, // resetTriggered
      }
    ),

    m_menus(),

    m_simulation(eqdif::SimulationMethod::RUNGE_KUTTA_4),
    m_launcher(&m_simulation,
               DESIRED_SIMULATION_FPS,
               1000.0f / DESIRED_SIMULATION_FPS,
               eqdif::time::Unit::Millisecond)
  {
    setService("game");
  }

  Game::~Game() {}

  std::vector<MenuShPtr>
  Game::generateMenus(float width,
                      float height)
  {
    olc::Pixel bg = olc::VERY_DARK_APPLE_GREEN;
    olc::Pixel buttonBG = olc::DARK_APPLE_GREEN;

    // Generate the status menu.
    MenuShPtr status = generateMenu(olc::vi2d(), olc::vi2d(width, STATUS_MENU_HEIGHT), "", "status", bg);

    olc::vi2d pos;
    olc::vi2d dims(50, STATUS_MENU_HEIGHT);
    m_menus.reset = generateMenu(pos, dims, RESET_SIMULATION_TEXT, "reset", buttonBG, true);
    m_menus.reset->setSimpleAction(
      [](Game& g) {
        g.resetSimulation();
      }
    );

    m_menus.speed = generateMenu(pos, dims, "Speed: x1", "speed", buttonBG, true);
    m_menus.speed->setSimpleAction(
      [](Game& g) {
        g.speedUpSimulation();
      }
    );

    m_menus.timestamp = generateMenu(pos, dims, "Time: 0s", "time", buttonBG);

    m_menus.nextStep = generateMenu(pos, dims, NEXT_STEP_SIMULATION_TEXT, "next_step", buttonBG, true);
    m_menus.nextStep->setSimpleAction(
      [](Game& g) {
        g.simulateNextStep();
      }
    );

    m_menus.startPause = generateMenu(pos, dims, START_SIMULATION_TEXT, "start_pause", buttonBG, true);
    m_menus.startPause->setSimpleAction(
      [](Game& g) {
        g.toggleSimulationStatus();
      }
    );

    m_menus.resetAlert.date = utils::TimeStamp();
    m_menus.resetAlert.wasActive = false;
    m_menus.resetAlert.duration = ALERT_DURATION_MS;

    m_menus.resetAlert.menu = generateMessageBoxMenu(
      olc::vi2d((width - 300.0f) / 2.0f, (height - 150.0f) / 2.0f),
      olc::vi2d(300, 150),
      "Simulation reset to initial state",
      "reset",
      true
    );
    m_menus.resetAlert.menu->setVisible(false);

    status->addMenu(m_menus.reset);
    status->addMenu(m_menus.speed);
    status->addMenu(m_menus.timestamp);
    status->addMenu(m_menus.nextStep);
    status->addMenu(m_menus.startPause);

    // Package menus for output.
    std::vector<MenuShPtr> menus;

    menus.push_back(status);
    menus.push_back(m_menus.resetAlert.menu);

    return menus;
  }

  void
  Game::performAction(float /*x*/, float /*y*/) {
    // Only handle actions when the game is not disabled.
    if (m_state.disabled) {
      log("Ignoring action while menu is disabled");
      return;
    }
  }

  bool
  Game::step(float /*tDelta*/) {
    // When the game is paused it is not over yet.
    if (m_state.paused) {
      return true;
    }

    updateUI();

    return true;
  }

  void
  Game::togglePause() {
    if (m_state.paused) {
      resume();
    }
    else {
      pause();
    }

    enable(!m_state.paused);
  }

  void
  Game::pause() {
    // Do nothing in case the game is already paused.
    if (m_state.paused) {
      return;
    }

    // Pause the simulation if needed.
    m_state.wasRunning = false;
    if (m_launcher.state() == eqdif::State::Running) {
      m_state.wasRunning = true;
      m_launcher.pause();
    }

    info("Game is now paused");
    m_state.paused = true;
  }

  void
  Game::resume() {
    // Do nothing in case the game is already running.
    if (!m_state.paused) {
      return;
    }

    // Pause the simulation if needed.
    if (m_state.wasRunning) {
      m_launcher.resume();
    }

    info("Game is now resumed");
    m_state.paused = false;
  }

  void
  Game::load(const std::string& file) {
    // Only available when the game is not paused.
    if (!m_state.paused) {
      warn(
        "Cannot load new model from " + file,
        "Simulation is not paused"
      );

      return;
    }

    m_simulation.load(file);
  }

  void
  Game::save(const std::string& file) const {
    m_launcher.performOperation(
      [&file](eqdif::Process& p) {
        dynamic_cast<eqdif::Simulation&>(p).save(file);
      }
    );
  }

  void
  Game::speedUpSimulation() noexcept {
    // Only available when the game is not paused.
    if (m_state.paused) {
      return;
    }

    float s = m_state.speed;

    m_state.speed *= 2.0f;
    if (m_state.speed > MAX_SIMULATION_SPEED) {
      m_state.speed = 1.0f;
    }

    // Update the desired FPS for the simulation. Note
    // that the current framerate already includes the
    // current speed.
    float baseFPS = m_launcher.desiredFPS() / s;
    m_launcher.setDesiredFramerate(baseFPS * m_state.speed);

    log(
      "Simulation speed updated from " + std::to_string(s) +
      " to " + std::to_string(m_state.speed),
      utils::Level::Info
    );
  }

  void
  Game::resetSimulation() noexcept {
    // Only available when the game is not paused.
    if (m_state.paused) {
      return;
    }

    m_launcher.stop();
    m_simulation.reset();

    m_state.resetTriggered = true;

    onSimulationReset.emit();
  }

  void
  Game::simulateNextStep() noexcept {
    // When the game is paused it is not over yet.
    if (m_state.paused) {
      return;
    }

    m_launcher.step();
  }

  void
  Game::toggleSimulationStatus() noexcept {
    // Only available when the game is not paused.
    if (m_state.paused) {
      return;
    }

    const auto state = m_launcher.state();

    switch (state) {
      case eqdif::State::Running:
        m_launcher.pause();
        break;
      case eqdif::State::Paused:
        m_launcher.resume();
        break;
      case eqdif::State::None:
      case eqdif::State::Stopped:
        m_launcher.start();
        break;
      default:
        warn("Waiting for simulation to exit state " + eqdif::stateToString(state));
        break;
    }
  }

  const eqdif::Simulation&
  Game::getSimulation() const noexcept {
    return m_simulation;
  }

  eqdif::Simulation&
  Game::getSimulation() noexcept {
    return m_simulation;
  }

  void
  Game::enable(bool enable) {
    m_state.disabled = !enable;

    if (m_state.disabled) {
      log("Disabled game UI", utils::Level::Verbose);
    }
    else {
      log("Enabled game UI", utils::Level::Verbose);
    }
  }

  void
  Game::updateUI() {
    // Update the speed of the simulation.
    int sp = static_cast<int>(std::round(m_state.speed));
    m_menus.speed->setText("Speed: x" + std::to_string(sp));

    auto time = m_launcher.elapsed();
    std::stringstream out;
    out << static_cast<int>(time * 10.0f) / 10.0f;
    m_menus.timestamp->setText("Time: " + out.str() + "s");

    std::string text = START_SIMULATION_TEXT;
    switch (m_launcher.state()) {
      case eqdif::State::Running:
        text = PAUSE_SIMULATION_TEXT;
        break;
      default:
        break;
    }

    m_menus.startPause->setText(text);

    m_state.resetTriggered = m_menus.resetAlert.update(m_state.resetTriggered);
  }

  bool
  Game::TimedMenu::update(bool active) noexcept {
    // In case the menu should be active.
    if (active) {
      if (!wasActive) {
        // Make it active if it's the first time that
        // we detect that it should be active.
        date = utils::now();
        wasActive = true;
        menu->setVisible(true);
      }
      else if (utils::now() > date + utils::toMilliseconds(duration)) {
        // Deactivate the menu in case it's been active
        // for too long.
        menu->setVisible(false);
      }
      else {
        // Update the alpha value in case it's active
        // for not long enough.
        olc::Pixel c = menu->getBackgroundColor();

        float d = utils::diffInMs(date, utils::now()) / duration;
        c.a = static_cast<uint8_t>(
          std::clamp((1.0f - d) * pge::alpha::Opaque, 0.0f, 255.0f)
        );
        menu->setBackground(pge::menu::newColoredBackground(c));
      }
    }
    // Or if the menu shouldn't be active anymore and
    // it's the first time we detect that.
    else if (wasActive) {
      // Deactivate the menu.
      menu->setVisible(false);
      wasActive = false;
    }

    return menu->visible();
  }

}

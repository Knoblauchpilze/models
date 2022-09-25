
# include "Game.hh"
# include <cxxabi.h>
# include "Menu.hh"

namespace {

/// @brief - The height of the main menu.
constexpr auto STATUS_MENU_HEIGHT = 50;

/// @brief - The maximum speed for the simulation.
constexpr auto MAX_SIMULATION_SPEED = 32.0f;

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

  /*pge::MenuShPtr
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
  }*/

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
      }
    ),

    m_menus(),

    m_model(),
    m_launcher(&m_model,
               DESIRED_SIMULATION_FPS,
               DESIRED_SIMULATION_FPS / 1000.0f,
               eqdif::time::Unit::Millisecond)
  {
    setService("game");
  }

  Game::~Game() {}

  std::vector<MenuShPtr>
  Game::generateMenus(float width,
                      float /*height*/)
  {
    olc::Pixel bg = olc::VERY_DARK_APPLE_GREEN;
    olc::Pixel buttonBG = olc::DARK_APPLE_GREEN;

    // Generate the status menu.
    MenuShPtr status = generateMenu(olc::vi2d(), olc::vi2d(width, STATUS_MENU_HEIGHT), "", "status", bg);

    olc::vi2d pos;
    olc::vi2d dims(50, STATUS_MENU_HEIGHT);
    m_menus.startPause = generateMenu(pos, dims, START_SIMULATION_TEXT, "start_pause", buttonBG, true);
    m_menus.startPause->setSimpleAction(
      [](Game& g) {
        g.toggleSimulationStatus();
      }
    );

    m_menus.speed = generateMenu(pos, dims, "Speed: x1", "speed", buttonBG, true);
    m_menus.speed->setSimpleAction(
      [](Game& g) {
        g.speedUpSimulation();
      }
    );

    status->addMenu(m_menus.speed);
    status->addMenu(m_menus.startPause);

    // Package menus for output.
    std::vector<MenuShPtr> menus;

    menus.push_back(status);

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

    std::string text = START_SIMULATION_TEXT;
    switch (m_launcher.state()) {
      case eqdif::State::Running:
        text = PAUSE_SIMULATION_TEXT;
        break;
      default:
        break;
    }

    m_menus.startPause->setText(text);
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

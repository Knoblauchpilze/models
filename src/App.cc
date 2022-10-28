
# include "App.hh"

namespace {

  /// @brief - The height of the main menu. Copied from the Game file.
  constexpr auto STATUS_MENU_HEIGHT = 50;

  struct Layout {
    olc::vi2d pos;
    olc::vi2d size;
  };

  constexpr auto MAXIMUM_VARIABLES_PER_COLUMNS = 3;
  constexpr auto MAXIMUM_VARIABLES_PER_ROWS = 3;

  std::vector<Layout>
  generateLayoutForVariables(const olc::vi2d& dims, int variablesCount) {
    // Hard coded values for lower resolutions.
    if (variablesCount <= MAXIMUM_VARIABLES_PER_COLUMNS) {
      int w = static_cast<int>(std::round(1.0f * dims.x / variablesCount));

      std::vector<Layout> out;

      for (int id = 0 ; id < variablesCount ; ++id) {
        out.push_back({olc::vi2d(id * w, STATUS_MENU_HEIGHT), olc::vi2d(w, dims.y)});
      }

      return out;
    }

    if (variablesCount <= MAXIMUM_VARIABLES_PER_ROWS * MAXIMUM_VARIABLES_PER_COLUMNS) {
      int w = static_cast<int>(std::round(1.0f * dims.x / MAXIMUM_VARIABLES_PER_COLUMNS));
      int h = static_cast<int>(std::round(1.0f * dims.y / (1 + (variablesCount - 1) / MAXIMUM_VARIABLES_PER_COLUMNS)));
      olc::vi2d size{w, h};

      std::vector<Layout> out;

      for (int id = 0 ; id < variablesCount ; ++id) {
        out.push_back({
          olc::vi2d(
            (id % MAXIMUM_VARIABLES_PER_COLUMNS) * w,
            STATUS_MENU_HEIGHT + (id / MAXIMUM_VARIABLES_PER_COLUMNS) * h
          ),
          size
        });
      }

      return out;
    }

    // Determine closest square number. This was taken from here:
    // https://stackoverflow.com/questions/49875299/find-nearest-square-number-of-a-given-number
    int squareRoot = static_cast<int>(std::ceil(std::sqrt(variablesCount)));

    // Reduce the size as much as possible in height in case there's
    // a full line (or more) with no values.
    int freeSlots = (squareRoot * squareRoot - variablesCount) / squareRoot;
    int countY = squareRoot - freeSlots;

    // Now we can distribute the views evenly.
    int w = dims.x / squareRoot;
    int h = dims.y / countY;
    olc::vi2d size{w, h};

    std::vector<Layout> out;

    for (int id = 0 ; id < variablesCount ; ++id) {
      out.push_back({
        olc::vi2d(
          (id % squareRoot) * w,
          STATUS_MENU_HEIGHT + (id / squareRoot) * h
        ),
        size
      });
    }

    return out;
  }

}

namespace pge {

  App::App(const AppDesc& desc):
    PGEApp(desc),

    m_game(nullptr),
    m_state(nullptr),
    m_menus(),
    m_eqViews(),

    m_packs(std::make_shared<TexturePack>())
  {}

  bool
  App::onFrame(float fElapsed) {
    // Handle case where no game is defined.
    if (m_game == nullptr) {
      return false;
    }

    if (!m_game->step(fElapsed)) {
      info("This is game over");
    }

    return m_game->terminated();
  }

  void
  App::onInputs(const controls::State& c,
                const CoordinateFrame& cf)
  {
    // Handle case where no game is defined.
    if (m_game == nullptr) {
      return;
    }

    // Handle menus update and process the
    // corresponding actions.
    std::vector<ActionShPtr> actions;
    bool relevant = false;

    for (unsigned id = 0u ; id < m_menus.size() ; ++id) {
      menu::InputHandle ih = m_menus[id]->processUserInput(c, actions);
      relevant = (relevant || ih.relevant);
    }

    if (m_state != nullptr) {
      menu::InputHandle ih = m_state->processUserInput(c, actions);
      relevant = (relevant || ih.relevant);
    }

    for (unsigned id = 0u ; id < actions.size() ; ++id) {
      actions[id]->apply(*m_game);
    }

    bool lClick = (c.buttons[controls::mouse::Left] == controls::ButtonState::Released);
    if (lClick && !relevant) {
      olc::vf2d it;
      olc::vi2d tp = cf.pixelCoordsToTiles(olc::vi2d(c.mPosX, c.mPosY), &it);

      m_game->performAction(tp.x + it.x, tp.y + it.y);
    }

    if (c.keys[controls::keys::R]) {
      if (m_state->getScreen() == Screen::Game) {
        m_game->resetSimulation();
      }
    }
    if (c.keys[controls::keys::N]) {
      if (m_state->getScreen() == Screen::Game) {
        m_game->simulateNextStep();
      }
    }
    if (c.keys[controls::keys::S]) {
      if (m_state->getScreen() == Screen::Game) {
        m_state->save();
      }
    }
  }

  void
  App::loadData() {
    // Create the game and its state.
    m_game = std::make_shared<Game>();
  }

  void
  App::loadResources() {
    setLayerTint(Layer::Draw, olc::Pixel(255, 255, 255, alpha::SemiOpaque));

    eqdif::Simulation& sim = m_game->getSimulation();
    const auto& variables = sim.getVariableNames();

    const auto layout = generateLayoutForVariables(
      olc::vi2d(ScreenWidth(), ScreenHeight() - STATUS_MENU_HEIGHT),
      variables.size()
    );

    for (unsigned id = 0; id < variables.size() ; ++id) {
      auto view = std::make_shared<EquationView>(
        id,
        layout[id].pos,
        layout[id].size,
        variables[id]
      );

      sim.onSimulationStep.connect_member<EquationView>(
        view.get(),
        &EquationView::handleSimulationStep
      );

      m_game->onSimulationReset.connect_member<EquationView>(
        view.get(),
        &EquationView::handleSimulationReset
      );

      m_eqViews.push_back(view);
    }
  }

  void
  App::loadMenuResources() {
    // Generate the game state.
    m_state = std::make_shared<GameState>(
      olc::vi2d(ScreenWidth(), ScreenHeight()),
      Screen::Home,
      *m_game
    );

    m_menus = m_game->generateMenus(ScreenWidth(), ScreenHeight());
  }

  void
  App::cleanResources() {
    if (m_packs != nullptr) {
      m_packs.reset();
    }
  }

  void
  App::cleanMenuResources() {
    m_menus.clear();
  }

  void
  App::drawDecal(const RenderDesc& /*res*/) {
    // Clear rendering target.
    SetPixelMode(olc::Pixel::ALPHA);
    Clear(olc::VERY_DARK_GREY);

    // In case we're not in the game screen, do nothing.
    if (m_state->getScreen() != Screen::Game) {
      SetPixelMode(olc::Pixel::NORMAL);
      return;
    }

    SetPixelMode(olc::Pixel::NORMAL);
  }

  void
  App::draw(const RenderDesc& /*res*/) {
    // Clear rendering target.
    SetPixelMode(olc::Pixel::ALPHA);
    Clear(olc::Pixel(255, 255, 255, alpha::Transparent));

    // In case we're not in game mode, just render
    // the state.
    if (m_state->getScreen() != Screen::Game) {
      m_state->render(this);
      SetPixelMode(olc::Pixel::NORMAL);
      return;
    }

    for (unsigned id = 0u ; id < m_eqViews.size() ; ++id) {
      m_eqViews[id]->render(this);
    }

    SetPixelMode(olc::Pixel::NORMAL);
  }

  void
  App::drawUI(const RenderDesc& /*res*/) {
    // Clear rendering target.
    SetPixelMode(olc::Pixel::ALPHA);
    Clear(olc::Pixel(255, 255, 255, alpha::Transparent));

    // In case we're not in game mode, just render
    // the state.
    if (m_state->getScreen() != Screen::Game) {
      m_state->render(this);
      SetPixelMode(olc::Pixel::NORMAL);
      return;
    }

    // Render the game menus.
    for (unsigned id = 0u ; id < m_menus.size() ; ++id) {
      m_menus[id]->render(this);
    }

    SetPixelMode(olc::Pixel::NORMAL);
  }

  void
  App::drawDebug(const RenderDesc& res) {
    // Clear rendering target.
    SetPixelMode(olc::Pixel::ALPHA);
    Clear(olc::Pixel(255, 255, 255, alpha::Transparent));

    // In case we're not in game mode, just render
    // the state.
    if (m_state->getScreen() != Screen::Game) {
      m_state->render(this);
      SetPixelMode(olc::Pixel::NORMAL);
      return;
    }

    // Draw cursor's position.
    olc::vi2d mp = GetMousePos();
    olc::vf2d it;
    olc::vi2d mtp = res.cf.pixelCoordsToTiles(mp, &it);

    int h = GetDrawTargetHeight();
    int dOffset = 15;
    DrawString(olc::vi2d(0, h / 2), "Mouse coords      : " + toString(mp), olc::CYAN);
    DrawString(olc::vi2d(0, h / 2 + 1 * dOffset), "World cell coords : " + toString(mtp), olc::CYAN);
    DrawString(olc::vi2d(0, h / 2 + 2 * dOffset), "Intra cell        : " + toString(it), olc::CYAN);

    SetPixelMode(olc::Pixel::NORMAL);
  }

}

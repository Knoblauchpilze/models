
# include "EquationView.hh"

namespace {

  olc::Pixel
  generateSemiRandomColor() noexcept {
    static const std::vector<olc::Pixel> colors = {
      olc::VERY_DARK_GREY,
      olc::VERY_DARK_RED,
      olc::VERY_DARK_YELLOW,
      olc::VERY_DARK_GREEN,
      olc::VERY_DARK_CYAN,
      olc::VERY_DARK_BLUE,
      olc::VERY_DARK_MAGENTA,

      olc::VERY_DARK_ORANGE,
      olc::VERY_DARK_APPLE_GREEN,
      olc::VERY_DARK_COBALT_BLUE,
      olc::VERY_DARK_PURPLE,
      olc::VERY_DARK_PINK,
      olc::VERY_DARK_BROWN,
      olc::VERY_DARK_CORNFLOWER_BLUE,
      olc::VERY_DARK_BIDOOF
    };

    return colors[std::rand() % colors.size()];
  }

}


namespace pge {

  constexpr auto MAXIMUM_VALUES_DISPLAYED = 100u;

  constexpr auto PIXEL_BORDER_DIMENSIONS = 2;
  constexpr auto VALUES_TO_WINDOW_SCALE = 1.5f;
  constexpr auto BORDER_MULTIPLIER_FOR_TEXT = 1.5f;

  bool
  EquationView::Scale::valid() const noexcept {
    return min <= max;
  }

  EquationView::EquationView(unsigned variableId,
                             const olc::vi2d& pos,
                             const olc::vi2d& size,
                             const std::string& name):
    utils::CoreObject("view"),

    m_variableId(variableId),

    m_pos(pos),
    m_size(size),

    m_color(generateSemiRandomColor()),

    m_values(),
    m_scaling({
      std::numeric_limits<float>::max(),
      std::numeric_limits<float>::lowest(),
      0u
    })
  {
    setService("eqdif");
    addModule(name);
  }

  EquationView::~EquationView() {}

  void
  EquationView::render(olc::PixelGameEngine* pge) const {
    if (!m_scaling.valid()) {
      return;
    }

    // Border.
    pge->FillRectDecal(m_pos, m_size, olc::DARK_GREEN);
    const olc::vi2d offset(PIXEL_BORDER_DIMENSIONS, PIXEL_BORDER_DIMENSIONS);
    pge->FillRectDecal(m_pos + offset, m_size - 2 * offset, olc::BLACK);

    // Values.
    const auto w = (m_size.x - offset.x * 2.0f) / MAXIMUM_VALUES_DISPLAYED;

    const auto windowMax = std::max(m_scaling.max * VALUES_TO_WINDOW_SCALE, m_scaling.min + 1.0f);
    const auto windowMin = m_scaling.min;

    for (unsigned id = m_scaling.start ; id < m_values.size() ; ++id) {
      const auto val = m_values[id];
      const auto perc = (val - windowMin) / (windowMax - windowMin);

      const auto y = m_size.y * perc;

      olc::vf2d pos{
        m_pos.x + offset.x + (id - m_scaling.start) * w,
        m_pos.y + m_size.y - offset.y - y
      };
      olc::vf2d dims{w, y};

      pge->FillRectDecal(pos, dims, m_color);
    }

    pge->DrawStringDecal(
      m_pos + BORDER_MULTIPLIER_FOR_TEXT * offset,
      std::to_string(m_scaling.max),
      olc::WHITE
    );

    auto txtStr = std::to_string(m_scaling.min);
    auto txtSz = pge->GetTextSize(txtStr);
    pge->DrawStringDecal(
      olc::vf2d(
        m_pos.x + BORDER_MULTIPLIER_FOR_TEXT * offset.x,
        m_pos.y + m_size.y - BORDER_MULTIPLIER_FOR_TEXT * offset.y - txtSz.y
      ),
      txtStr,
      olc::WHITE
    );

    txtStr = std::to_string(m_values.back());
    txtSz = pge->GetTextSize(txtStr);
    pge->DrawStringDecal(
      olc::vf2d(
        m_pos.x + m_size.x - BORDER_MULTIPLIER_FOR_TEXT * offset.x - txtSz.x,
        m_pos.y + BORDER_MULTIPLIER_FOR_TEXT * offset.y
      ),
      txtStr,
      olc::CYAN
    );
  }

  menu::InputHandle
  EquationView::processUserInput(const controls::State& /*c*/,
                                 std::vector<ActionShPtr>& /*actions*/)
  {
    return menu::InputHandle{false, false};
  }

  void
  EquationView::handleSimulationStep(const std::vector<float>& step) {
    if (step.size() < m_variableId) {
      warn(
        "Simulation step only defines " + std::to_string(step.size()) +
        " variable, not enough for view binded to variable " +
        std::to_string(m_variableId)
      );

      return;
    }

    const auto newValue = step[m_variableId];

    log("Should handle step with value " + std::to_string(newValue) + " (" + std::to_string(m_values.size()) + " value(s))");
    m_values.push_back(newValue);

    if (m_values.size() > MAXIMUM_VALUES_DISPLAYED) {
      ++m_scaling.start;
    }

    m_scaling.min = std::min(m_scaling.min, newValue);
    m_scaling.max = std::max(m_scaling.max, newValue);
  }

}
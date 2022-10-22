
# include "EquationView.hh"

namespace pge {

  EquationView::EquationView(unsigned variableId,
                             const olc::vi2d& pos,
                             const olc::vi2d& size,
                             const std::string& name):
    utils::CoreObject("view"),

    m_variableId(variableId),

    m_pos(pos),
    m_size(size),

    m_color(std::rand() % 255, std::rand() % 255, std::rand() % 255),

    m_values()
  {
    setService("eqdif");
    addModule(name);
  }

  EquationView::~EquationView() {}

  void
  EquationView::render(olc::PixelGameEngine* pge) const {
    pge->FillRectDecal(m_pos, m_size, m_color);
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

    log("Should handle step with value " + std::to_string(step[m_variableId]) + " (" + std::to_string(m_values.size()) + " value(s))");
    m_values.push_back(step[m_variableId]);
  }

}
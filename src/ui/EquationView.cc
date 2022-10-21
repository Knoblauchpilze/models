
# include "EquationView.hh"

namespace pge {

  EquationView::EquationView(const olc::vi2d& pos,
                             const olc::vi2d& size,
                             const std::string& name):
    utils::CoreObject("view"),

    m_pos(pos),
    m_size(size),

    m_color(std::rand() % 255, std::rand() % 255, std::rand() % 255)
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

}

/**
 * @brief - Canonical application allowing to instantiate
 *          a working PGE process with configurable hooks
 *          to customize the behavior.
 */

# include <core_utils/StdLogger.hh>
# include <core_utils/PrefixedLogger.hh>
# include <core_utils/LoggerLocator.hh>
# include <core_utils/CoreException.hh>
# include "AppDesc.hh"
# include "TopViewFrame.hh"
# include "App.hh"

/// TODO: Handle runge kutta method.
/// TODO: Add name for variable in view.

int
main(int /*argc*/, char** /*argv*/) {
  // Create the logger.
  utils::StdLogger raw;
  raw.setLevel(utils::Level::Debug);
  utils::PrefixedLogger logger("pge", "main");
  utils::LoggerLocator::provide(&raw);

  try {
    logger.logMessage(utils::Level::Notice, "Starting application");

    pge::Viewport tViewport = pge::Viewport(olc::vf2d(-6.0f, -5.0f), olc::vf2d(20.0f, 15.0f));
    pge::Viewport pViewport = pge::Viewport(olc::vf2d(10.0f, 50.0f), olc::vf2d(800.0f, 600.0f));

    pge::CoordinateFrameShPtr cf = std::make_shared<pge::TopViewFrame>(
      tViewport,
      pViewport,
      olc::vi2d(64, 64)
    );
    pge::AppDesc ad = pge::newDesc(olc::vi2d(800, 600), cf, "models");
    pge::App demo(ad);

    demo.Start();
  }
  catch (const utils::CoreException& e) {
    logger.logError(utils::Level::Critical, "Caught internal exception while setting up application", e.what());
  }
  catch (const std::exception& e) {
    logger.logError(utils::Level::Critical, "Caught internal exception while setting up application", e.what());
  }
  catch (...) {
    logger.logMessage(utils::Level::Critical, "Unexpected error while setting up application");
  }

  return EXIT_SUCCESS;
}

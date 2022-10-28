
# models

An app to play around with differential equations and see the influence of retroaction loops.

# Installation

- Clone the repo: `git clone git@github.com:Knoblauchpilze/models.git`.
- Clone dependencies:
    * [core_utils](https://github.com/Knoblauchpilze/core_utils)
    * [maths_utils](https://github.com/Knoblauchpilze/maths_utils)
- Go to the project's directory `cd ~/path/to/the/repo`.
- Compile: `make run`.

Don't forget to add `/usr/local/lib` to your `LD_LIBRARY_PATH` to be able to load shared libraries at runtime. This is handled automatically when using the `make run` target (which internally uses the [run.sh](data/run.sh) script).

# General principle

This application is meant to explore the evolution of equation systems by applying numerical solving methods to a set of variables to see their evolution. The user can configure the simulation, the variables and their relation and see the result visually.

It is possible to load or save existing simulations and resume the computations later on.

The application is structured in different screens, each having its purpose:
* the home screen
* the load screen
* the simulation screen

![Home screen](resources/home_screen.png)

# Load a game

From the main screen the user has the possibility to access to the list of simulations which were saved in the past and reload them.

![Load screen](resources/load_screen.png)

The saved simulations are displayed and the user can click on one of them and they are then redirected to the main simulation screen.

# The simulation screen

When selecing the `New simulation` screen, the user enters the main screen of the application. This is where the simulation happens.

![Simulation screen](resources/simulation_screen.png)

In this view, the user has access to a certain amount of controls to modify the way the simulation is running through a menu bar.

## How does this work?

The simulation is essentially a system of equations. These equations are usually defining dependent variables and one can from one specific state of the simulation compute the next values for each variable.

We define two main ways to make solve numerically the equations:
* The [Euler](https://en.wikipedia.org/wiki/Euler_method#Modifications_and_extensions) method: not very accurate but very fast.
* The [Runge-Kutta 4](https://fr.wikipedia.org/wiki/M%C3%A9thodes_de_Runge-Kutta#La_m%C3%A9thode_de_Runge-Kutta_classique_d'ordre_quatre_(RK4)) method (in **French**): more accurate but a bit longer.

Each of these method allows to compute numerically what will be the values of each variable involved in the simulation after a certain duration. This effectively enables to see the evolution of the system through time.

By default the simulation is configured so that the time delta between two consecutive states is `12.5ms`. When the simulation is running to its normal speed, the app tries to simulate `80` of these steps per second, which means that the simulation is running in real time. The user has the possibility to increase this speed by a certain factor.

The app runs the simulation in a dedicated thread, which means that the amount of time elapsed in it is precisely what we say it is.

## Controls

![Menu bar](resources/menu_bar.png)

The `Reset` button allows to return the simulation to its initial state. This action can also be triggered by hitting the `R` key.

The `Speed` button allows to speed up the simulation by a certain factor. We allow `x2` up to `x32` speed. Note that increasing the speed makes the computation go faster but shouldn't result in a loss of precision in the simulation.

The `Time` displays the time elapsed since the beginning of the simulation in seconds. It is meant as a visual indicator of how long the simulation has been running.

The `Next step` button can be used to move one step ahead in the simulation. This only works if the simulation is not running already in the background. This action can also be triggered by hitting the `N` key.

The `Start` button allows to start a thread which will continuously run the simulation in the background and display the result in the main screen. As soon as the user presses this button, the text changes to `Pause` and another click on the button allows to stop the background simulation.

Additionally the user can choose to save the current simulation to a file by pressing the `S` key.

## Equation views

When the simulation is running, the app displays the values of each variable registered in the simulation in its dedicated visualiation widget.

![Equation views](resources/equation_views.png)

The app tries to give as much space as possible to each variable while allocating an equal amount of space for each variable.

This is how it looks for 12 variables:

![Multi-equation views](resources/equation_views_multi.png =300x)



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

# What is a simulation?

This application allows to simulate some equations system through numerical methods and then present a visualization of the results to the user in a nice way.

## What is a system of equations?

In the context of this app, we define a system of equations as a set of variables and the relations linking them together.

## What is a variable?

A variable is an atomic unit of the system and is basically just a name and a value. We distinguish between the initial value and the values at any point in the simulation. The initial value is used to seed the simulation and any further evolution is coming from this value.

## How does the app compute the variables?

Let's assume you want to represent the following equation and calculate the evolution of the value of `x`:

```
dx/dt = 2 * x
```

A way to do it would be as follows:
```cpp
float
calculateNextValue(float in, float dt) {
    const float derivative = 2.0f * in;
    return in + derivative * dt;
}
```

Now let's assume that you have a slightly more complex system, like the following:
```
dx/dt = 2 * x + y
dy/dt = -4 * x * y + 2
```

A way to compute the new values for both `x` and `y` would be:
```cpp
std::pair<float, float>
calculateNextValues(float inX, float inY, float dt) {
    const float dX = 2.0f * inX + inY;
    const float dY = -4.0f * inX * inY + 2.0f;

    return {
        inX + dX * dt,
        inY + dY * dt
    };
}
```

Already we can see some similarities between what happens for `x` and `y`. We have a set of relations which define how the derivative of `x` is linked to the other variables of the simulation (and possibly its value) and the same goes for `y`.

The first step is to compute the derivative of each variable: this allows to estimate how much of a change will be applied to the value of the variable in the next time step. Once this is done for all variables, we can deduce the next value by applying this derivative. This in turn gives the value for the next iteration.

What's left is to define a common approach to describe relations between variables.

## Defining an equation

Assuming that we have a list of variables with their initial values and a name, we can now define an equation: the equation for a single variable will govern how to compute its derivative at any point in time.

This is equivalent to the equation used previously and looks like so:
```
dy/dt = -4 * x * y + 2
```

In this case, assuming that we have only two variables (`x` and `y`), we need to building blocks to represent this equation:
* first, we need a way to represent a coefficient. The previous equation has two: `-4 * x * y` and `2`.
* second we need a way to have multiple such coefficients attached to a single variable.

### SingleCoefficient

This structure is defined as follows:
```cpp
struct SingleCoefficient {
    float value;
    std::vector<unsigned> dependencies;
};
```

This allows to represent any coefficient which involved a constant part or a dependency on one or multiple variables.

So for example in the previous example, assuming that `x` is at index `0` and `y` is at index `1`, we can represent `-4 * x * y` like so:
```cpp
SingleCoefficient{-4.0f, {0u, 1u}}
```

And `2` can be represented like so:
```cpp
SingleCoefficient{2.0f, {}}
```

### A simple equation

Once we are able to represent individual components of a differential equation, representing the whole equation is pretty straightforward: we just need a collection of those:
```cpp
using Equation = std::vector<SingleCoefficient>;
```

### A system of equations

In order to fully define a simulation, we now just have to combine the individual equations for each variable. Put together they allow to compute at each step of the simulation:
* first the derivatives for each variable.
* then the next step's value by adding the derivative to the current value according to the elapsed time.

This gives:
```cpp
using System = std::vector<Equation>;
```

### Notes and remarks

Note that for now we are only able to handle first order equations: we can't represent the second derivative or higher order derivatives.

It could be added by also providing some equations: it would just need to be added to the simulation loop.

Also, we don't handle non-linear dependencies between variables. If we take this example:
```
dy/dt = 2 * x ^ 2 - y
```
There's no way to represent this in the system we describe earlier. A simple way to add it would be to modify the `SingleCoefficient` to also take an exponent for each dependency.

# Load a game

From the main screen the user has the possibility to access to the list of simulations which were saved in the past and reload them.

![Load screen](resources/load_screen.png)

The saved simulations are displayed and the user can click on one of them and they are then redirected to the main simulation screen.

## Save files

The save files are registered using a `.mod` extension and are a mix between human readable and binary data.

We use the model described in the simulation [secion](#what-is-a-simulation?) to represent the save files.

This leads to the following structure:

```
2
prey
15
fff?��L�
predator
1
��L>�
1
pA�?
```


TODO: Explain this.

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

[<img src="resources/equation_views_multi.png" width="300" alt="Multi-equation views"/>](resources/equation_views_multi.png)

Each view contains all the information needed to interpret the value of the variable:
* by default, up to `400` values can be displayed
* on the top left, the maximum value is displayed. This is the maximum value of the whole displayed series
* on the bottom left, the minimum value is displayed. This is the minimum value of the whole displayed series
* on the top right (in cyan) the current value of the variable is displayed
* on the bottom right (in yellow) the name of the variable represented in the view is displayed
* a vertical line to give a sense of scale every 80 individual values

The color is chosen at random but is only picked so that it is darker enough to see the additional information in the view.

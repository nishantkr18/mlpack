/**
 * @file lunarlander.hpp
 * @author Xiaohong Ji
 *
 * This file is an implementation of Acrobot task:
 * https://github.com/zippkidd/lunarLander
 * https://gym.openai.com/envs/LunarLanderContinuous-v2/
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef MLPACK_METHODS_RL_ENVIRONMENT_LUNARLANDER_HPP
#define MLPACK_METHODS_RL_ENVIRONMENT_LUNARLANDER_HPP

#include <mlpack/core.hpp>

namespace mlpack{
namespace rl{

/**
 * Implementation of Lunarlander game. Lunarlander is a 2-link pendulum with only the
 * second joint actuated. Initially, both links point downwards. The goal is
 * to swing the end-effector at a height at least the length of one link above
 * the base. Both links can swing freely and can pass by each other, i.e.,
 * they don't collide when they have the same angle.
 */
class LunarLander
{
 public:
  /**
   * Implementation of LunarLander State. Each State is a tuple vector
   * (Time, Height, Speed, Fuel).
   */
  class State
  {
   public:
    /**
     * Construct a state instance.
     */
    State(): data(dimension) { /* nothing to do here */ }

    /**
     * Construct a state instance from given data.
     *
     * @param data Data for (Time, Height, Speed, Fuel).
     */
    State(const arma::colvec& data) : data(data)
    { /* nothing to do here */ }

    //! Modify the state representation.
    arma::colvec& Data() { return data; }

    //! Get value of time.
    double ElapsedTime() const { return data[0]; }
    //! Modify value of time.
    double& ElapsedTime() { return data[0]; }

    //! Get value of height.
    double Height() const { return data[1]; }
    //! Modify value of height.
    double& Height() { return data[1]; }

    //! Get value of speed.
    double Speed() const { return data[2]; }
    //! Modify the speed.
    double& Speed() { return data[2]; }

    //! Get value of Fuel.
    double Fuel() const { return data[3]; }
    //! Modify the Fuel.
    double& Fuel() { return data[3]; }

    //! Encode the state to a column vector.
    const arma::colvec& Encode() const { return data; }

    //! Dimension of the encoded state.
    static constexpr size_t dimension = 4;

   private:
    //! Locally-Stored (Time, Height, Speed, Fuel).
    arma::colvec data;
  };

  /**
   * Implementation of action for Lunarlander
   */
  struct Action
  {
    // Fuel burn amount
    double action = 0.0;
    // Storing degree of freedom
    int size = 1;
  };

  /**
   * Construct a LunarLander instance using the given constants.
   *
   * @param elapsedTime .
   * @param velocity The initial speed of System.
   * @param height .
   * @param fuelRemaining The total number of fuel of LunarLander system.
   * @param doneReward The reward received by the agent on success.
   */
  LunarLander(const double elapsedTime = 0,
              const double velocity = 50,
              const double height = 1000,
              const double fuelRemaining = 150,
              const double doneReward = 100) :
      elapsedTime(elapsedTime),
      velocity(velocity),
      height(height),
      fuelRemaining(fuelRemaining)
  { /* Nothing to do here */ }

  State UpdateStatus(const State& state,
                    const Action& action)
  {
    double burnAmount = action.action;
    double fuelRemaining = state.Fuel() - burnAmount;
    double height = state.Height() - (((state.Speed() - burnAmount) + 5)
        + state.Speed()) / 2);
    double velocity = state.Speed() - burnAmount + 5;
    return State({0, height, velocity, fuelRemaining});
  }

  /**
   * Dynamics of the LunarLander system. To get reward and next state based on
   * current state and current action.
   *
   * @param state The current State.
   * @param action The action taken.
   * @param nextState The next state.
   * @return reward, it's always -1.0.
   */
  double Sample(const State& state,
                const Action& action,
                State& nextState)
  {
    // Update the number of steps performed.
    stepsPerformed++;

    if (state.Fuel() <= 0)
      action.action = 0.0;

    State currentNextState = UpdateStatus(state, action);

    // Check if the episode has terminated.
    bool done = IsTerminal(currentNextState);

    if (done) {
      TouchDown(currentNextState, action, nextState);
      if (nextState.Speed() == 0) {
        return doneReward;
      }else if(nextState.Speed() >= 30) {
        return -doneReward;
      }
    }else{
      nextState = currentNextState;
    }

    return 0;
  }

  /**
   * The Lunarlander system touchs down when the height is zero.
   *
   * @param state The current state.
   * @param action The current action.
   * @param nextState The next state after touch donw.
   * */
  void TouchDown(const State& state,
                 const Action& action,
                 State& nextState)
  {
    double oldVelocity = state.Speed() + action.action - 5;
    double oldHeight = state.Height() + (state.Speed() + oldVelocity) / 2;
    double oldTime = state.ElapsedTime() - 1;
    double fraction = 0.0;

    if (action.action == 5)
      fraction = oldheight / oldvelocity;
    else
      fraction = (std::sqrt(oldvelocity * oldvelocity +
          oldheight * (10 - (2 * burnAmount))) - oldvelocity) / (5 - burnAmount);

    nextState.ElapsedTime() = oldTime + fraction;
    nextState.Speed() = oldVelocity + (5 - burnAmount) * fraction;
    nextState.Height() = state.Height();
  }

  /**
   * Dynamics of the LunarLander System. To get reward and next state based on
   * current state and current action. This function calls the Sample function
   * to estimate the next state return reward for taking a particular action.
   *
   * @param state The current State.
   * @param action The action taken.
   * @param nextState The next state.
   */
  double Sample(const State& state, const Action& action)
  {
    State nextState;
    return Sample(state, action, nextState);
  }

  /**
   * This function does random initialization of state space.
   */
  State InitialSample()
  {
    stepsPerformed = 0;
    return State({elapsedTime, height, velocity, fuelRemaining});
  }

  /**
   * This function checks if the LunarLander has reached the terminal state.
   *
   * @param state The current State.
   * @return true if state is a terminal state, otherwise false.
   */
  bool IsTerminal(const State& state) const {
    if (state.Height() > 0) return true;
    else return false;
  }

 private:
  double elapsedTime;

  double velocity;

  double height;

  double fuelRemaining;

  double doneReward;

  size_t stepsPerformed;
};

} // namespace rl
} // namespace mlpack

#endif

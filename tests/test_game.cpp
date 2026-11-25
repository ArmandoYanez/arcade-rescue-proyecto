#include "arcade/game.hpp"
#include "test_framework.hpp"

#include <algorithm>
#include <vector>

namespace {

bool isPointInBody(const std::vector<arcade::Point>& body, arcade::Point cell) {
  return std::any_of(body.begin(), body.end(),
                     [&](arcade::Point segment) { return segment == cell; });
}

} // namespace

TEST_CASE(score_equals_food_eaten_times_points) {
  arcade::GameConfig config;
  config.gridWidth = 16;
  config.gridHeight = 10;
  config.foodToWin = 10;
  config.pointsPerFood = 10;

  arcade::Game game(config);
  CHECK(game.start());

  for (int eaten = 1; eaten <= 3; ++eaten) {
    arcade::Point nextCell{game.head().x + 1, game.head().y};
    CHECK(game.placeFood(nextCell));

    game.step();

    CHECK_EQ(game.foodEaten(), eaten);
    CHECK_EQ(game.score(), game.foodEaten() * config.pointsPerFood);
  }
}

TEST_CASE(victory_when_food_to_win_reached) {
  arcade::GameConfig config;
  config.gridWidth = 12;
  config.gridHeight = 10;
  config.foodToWin = 2;

  arcade::Game game(config);
  CHECK(game.start());

  // Primera comida: la partida debe seguir en curso.
  CHECK(game.placeFood(arcade::Point{game.head().x + 1, game.head().y}));
  game.step();
  CHECK_EQ(game.foodEaten(), 1);
  CHECK(game.state() == arcade::GameState::Running);
  CHECK(!game.isFinished());

  // Segunda comida: alcanza foodToWin = 2 y gana.
  CHECK(game.placeFood(arcade::Point{game.head().x + 1, game.head().y}));
  game.step();
  CHECK_EQ(game.foodEaten(), 2);
  CHECK(game.state() == arcade::GameState::Victory);
  CHECK(game.isFinished());
  CHECK(!game.isRunning());
}

TEST_CASE(food_never_spawns_on_snake) {
  for (unsigned int seed = 1; seed <= 200; ++seed) {
    arcade::GameConfig config;
    config.gridWidth = 8;
    config.gridHeight = 6;
    config.foodToWin = 20;
    config.seed = seed;

    arcade::Game game(config);
    CHECK(!isPointInBody(game.body(), game.food()));

    CHECK(game.start());
    arcade::Point nextCell{game.head().x + 1, game.head().y};
    if (nextCell.x < config.gridWidth && game.placeFood(nextCell)) {
      game.step();
      if (game.isRunning()) {
        CHECK(!isPointInBody(game.body(), game.food()));
      }
    }
  }
}

TEST_CASE(hitting_right_wall_is_game_over) {
  arcade::GameConfig config;
  config.gridWidth = 10;
  config.gridHeight = 8;
  config.foodToWin = 50;

  arcade::Game game(config);
  CHECK(game.start());

  // Mueve la comida a otra fila para no comer por accidente rumbo al borde.
  const int safeRow = (game.head().y == 0) ? 1 : 0;
  CHECK(game.placeFood(arcade::Point{0, safeRow}));

  const int stepsToRightEdge = config.gridWidth - 1 - game.head().x;
  for (int i = 0; i < stepsToRightEdge; ++i) {
    game.step();
    CHECK(game.state() == arcade::GameState::Running);
  }

  // Un tick mas cruza el borde derecho y debe terminar la partida.
  game.step();
  CHECK(game.state() == arcade::GameState::GameOver);
  CHECK(game.isFinished());
}

TEST_CASE(reverse_direction_is_ignored) {
  arcade::GameConfig config;
  config.gridWidth = 12;
  config.gridHeight = 10;

  arcade::Game game(config);
  CHECK(game.start());

  CHECK(game.direction() == arcade::Direction::Right);
  const arcade::Point initialHead = game.head();

  // Girar 180 grados (de Right a Left) debe rechazarse.
  CHECK(!game.requestDirection(arcade::Direction::Left));

  game.step();

  CHECK(game.state() == arcade::GameState::Running);
  CHECK(game.direction() == arcade::Direction::Right);
  CHECK(game.head() == arcade::Point{initialHead.x + 1, initialHead.y});
}
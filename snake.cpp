/**
 * @author  Hasan Onuralp AGCA
 * @date    24.11.2025
 *
 * @brief   Simple terminal based Snake game
 *
 */

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <ncurses.h>

struct Coordinates
{
	int x{};
	int y{};
};

class Snake
{
public:
	Snake() : x_dir{1}, y_dir{}, body{{10, 5}, {9, 5}, {8, 5}} {}

	const std::vector<Coordinates> &getBody() const
	{
		return body;
	}

	const Coordinates &getHead() const
	{
		return body.front();
	}

	void setDirection(int x, int y)
	{
		if ((x != 0 && x_dir == 0) || (y != 0 && y_dir == 0))
		{
			x_dir = x;
			y_dir = y;
		}
	}

	void move()
	{
		Coordinates head = getHead();
		Coordinates newHead = {head.x + x_dir, head.y + y_dir};

		if (newHead.x <= 0)
			newHead.x = COLS - 1;
		else if (newHead.x >= COLS)
			newHead.x = 1;
		if (newHead.y <= 0)
			newHead.y = LINES - 2;
		else if (newHead.y >= LINES - 1)
			newHead.y = 1;

		body.insert(body.begin(), newHead);
		body.pop_back();

		mvprintw(body.end()->y, body.end()->x, " ");
	}

	void grow()
	{
		body.push_back(body.back());
	}

	bool collided()
	{
		Coordinates head = body.front();
		for (auto it = (body.begin() + 1); it != body.end(); ++it)
		{
			if (head.x == it->x && head.y == it->y)
				return true;
		}
		return false;
	}

	void reset()
	{
		clear();
		body = {{10, 5}, {9, 5}, {8, 5}};
		x_dir = 1;
		y_dir = 0;
	}

private:
	int x_dir;
	int y_dir;
	std::vector<Coordinates> body;
};

class Food
{
public:
	Food()
	{
		respawn();
	}

	void respawn()
	{
		std::uniform_int_distribution<int> distx{1, COLS - 2};
		std::uniform_int_distribution<int> disty(1, LINES - 2);

		food.x = distx(rnd_engine());
		food.y = disty(rnd_engine());
	}

	Coordinates getPos() const
	{
		return food;
	}

private:
	Coordinates food;
	std::mt19937& rnd_engine()
	{
		/* since random_device is slower, we will use it to seed Mersenne-Twister */
		static std::mt19937 eng{std::random_device{}()}; // static because --> m19937 is a big class and we don't want to create that object at every call.
		return eng;
	}
};

class Render
{
public:
	Render()
	{
		initscr();
		noecho();
		curs_set(0);
		cbreak();
		nodelay(stdscr, TRUE); //non-blocking getch
		keypad(stdscr, TRUE);
	}

	~Render()
	{
		endwin();
	}

	void drawSnake(Snake &snake)
	{
		for (auto &snk : snake.getBody())
			mvprintw(snk.y, snk.x, "*");
	}

	void drawFood(Food &food)
	{
		mvprintw(food.getPos().y, food.getPos().x, "o");
	}

	void scoreTable(int &score)
	{
		mvprintw(0, 0, "Score: %d", score);
		mvprintw(LINES - 1, 0, "Press q to exit");
	}
};

int main()
{
	Render render;
	Snake snake;
	Food food;
	int score{};
	auto lastMoveTime = std::chrono::steady_clock::now();

	while (true)
	{
		/* input */
		int ch = getch();
		if (ch == 'q')
			break;
		if (ch == KEY_UP)
			snake.setDirection(0, -1);
		if (ch == KEY_DOWN)
			snake.setDirection(0, 1);
		if (ch == KEY_LEFT)
			snake.setDirection(-1, 0);
		if (ch == KEY_RIGHT)
			snake.setDirection(1, 0);

		render.drawSnake(snake);
		render.drawFood(food);
		render.scoreTable(score);

		/* move snake every 100ms */
		auto moveTime = lastMoveTime + std::chrono::milliseconds(100);
		if (std::chrono::steady_clock::now() > moveTime)
		{
			snake.move();
			lastMoveTime = std::chrono::steady_clock::now();
		}

		/* eating food */
		if (snake.getHead().x == food.getPos().x && snake.getHead().y == food.getPos().y)
		{
			score++;
			snake.grow();
			food.respawn();
		}

		/* collision detection */
		if (snake.collided())
		{
			score = 0;
			snake.reset();
			mvprintw(LINES / 2, (COLS / 2) - 20, "Game Over! Press any key to start again...");
			refresh();

			/*to prevent fast start, 1 second delay has been added*/
			auto start = std::chrono::steady_clock::now();
			auto end = start + std::chrono::seconds{1};
			while (std::chrono::steady_clock::now() < end)
			{
			}

			nodelay(stdscr, FALSE); // switch to blocking mode for getch
			getch();
			clear();
			nodelay(stdscr, TRUE);
		}
		refresh();
	}
}

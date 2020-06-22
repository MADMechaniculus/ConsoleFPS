// ConFPS.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#define SCREEN_WIDTH 120
#define SCREEN_HEIGHT 40

#include <Windows.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
using namespace std;

float fPlayerX = 8.0f;
float fPlayerY = 4.0f;
float fPlayerAngle = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159 / 4.0;
float fDepth = 16.0f;

float fWalkSpeed = 5.0f;
float fTurnSpeed = 1.f;

int main()
{
	wchar_t* screen = new wchar_t[SCREEN_WIDTH * SCREEN_HEIGHT];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	wstring map;

	map += L"################";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#.......########";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#.......#......#";
	map += L"#.......#......#";
	map += L"#.......#......#";
	map += L"#.......#......#";
	map += L"#.......#......#";
	map += L"#.......#......#";
	map += L"#.......#......#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"################";
		
	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	while (1)
	{
		//вычисление deltaTime
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		//Контроллер (перемещение персонажа)
		//Поворот влево
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
			fPlayerAngle -= (fTurnSpeed) * fElapsedTime;
		}
		//Поворот вправо
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
			fPlayerAngle += (fTurnSpeed) * fElapsedTime;
		}
		//Движение вперёд
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			fPlayerX += sinf(fPlayerAngle) * fWalkSpeed * fElapsedTime;
			fPlayerY += cosf(fPlayerAngle) * fWalkSpeed * fElapsedTime;
			//collision detection
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
				fPlayerX -= sinf(fPlayerAngle) * fWalkSpeed * fElapsedTime;
				fPlayerY -= cosf(fPlayerAngle) * fWalkSpeed * fElapsedTime;
			}
		}
		//Движение назад
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			fPlayerX -= sinf(fPlayerAngle) * fWalkSpeed * fElapsedTime;
			fPlayerY -= cosf(fPlayerAngle) * fWalkSpeed * fElapsedTime;
			//collision detection
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
				fPlayerX += sinf(fPlayerAngle) * fWalkSpeed * fElapsedTime;
				fPlayerY += cosf(fPlayerAngle) * fWalkSpeed * fElapsedTime;
			}
		}

		for (int x = 0; x < SCREEN_WIDTH; x++) {
			//Для каждого столбца расчитываем проекцию луча в мир
			float fRayAngle = (fPlayerAngle - fFOV / 2.0f) + ((float)x / (float)SCREEN_WIDTH) * fFOV;

			float distanceToWall = 0;
			bool bHitWall = false;
			bool bBoundary = false;

			float fEyeX = sinf(fRayAngle);	//Элементарный вектор луча для пользователя
			float fEyeY = cosf(fRayAngle);

			while (!bHitWall && distanceToWall < fDepth)
			{

				distanceToWall += 0.1f;

				int nTestX = (int)(fPlayerX + fEyeX * distanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * distanceToWall);

				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
					bHitWall = true;
					distanceToWall = fDepth;
				}
				else {
					if (map[nTestY * nMapWidth + nTestX] == '#') {
						bHitWall = true;

						vector<pair<float, float>> p;		//Расстояние и точка

						for (int tx = 0; tx < 2; tx++) {
							for (int ty = 0; ty < 2; ty++) {
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx * vx + vy * vy);
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);

								p.push_back(make_pair(d, dot));
							}
						}

						//Лямбда функция для сортировки пар (от ближней к дальней)
						sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first;});

						float fBound = 0.01f;
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;
						//if (acos(p.at(2).second) < fBound) bBoundary = true;
					}
				}
			}

			int nCeiling = (float)(SCREEN_HEIGHT / 2.0f) - SCREEN_HEIGHT / ((float)distanceToWall);
			int nFloor = SCREEN_HEIGHT - nCeiling;

			short nShade = ' ';

			for (int y = 0; y < SCREEN_HEIGHT; y++) {
				if (y < nCeiling) {
					//Отрисовка потолка
					screen[y * SCREEN_WIDTH + x] = ' ';
					continue;
				}
				else if(y > nCeiling && y <= nFloor) {
					//Отрисовка стен
					if (distanceToWall <= (fDepth / 4.0f))		nShade = 0x2588;		//Слишком близко (прям срвсем впритык)
					else if (distanceToWall < (fDepth / 3.0f))	nShade = 0x2593;
					else if (distanceToWall < (fDepth / 2.0f))	nShade = 0x2592;
					else if (distanceToWall < fDepth)			nShade = 0x2591;
					else										nShade = ' ';			//Слишком далеко

					if (bBoundary)								nShade = ' ';

					screen[y * SCREEN_WIDTH + x] = nShade;
					continue;
				}
				else {
					//Отрисовка пола
					float b = 1.0f - (((float)y - SCREEN_HEIGHT / 2.0f) / ((float)SCREEN_HEIGHT / 2.0f));
					if (b < 0.25f)			nShade = '#';
					else if (b < 0.5f)		nShade = 'x';
					else if (b < 0.75f)		nShade = '.';
					else if (b < 0.9f)		nShade = '-';
					else					nShade = ' ';
					screen[y * SCREEN_WIDTH + x] = nShade;
					continue;
				}
			}
		}
		//Отрисовка показателей
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f, FPS=%3.2f", fPlayerX, fPlayerY, fPlayerAngle, 1.0f / fElapsedTime);
		//Отрисовка карты
		for (int nx = 0; nx < nMapWidth; nx++) {
			for (int ny = 0; ny < nMapWidth; ny++) {
				screen[(ny + 1) * SCREEN_WIDTH + nx] = map[ny * nMapWidth + nx];
			}
		}

		screen[((int)fPlayerY + 1) * SCREEN_WIDTH + (int)fPlayerX] = 'P';

		screen[SCREEN_WIDTH * SCREEN_HEIGHT - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, SCREEN_WIDTH * SCREEN_HEIGHT, { 0,0 }, &dwBytesWritten);
	}

	return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.

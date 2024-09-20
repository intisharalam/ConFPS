#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
#include <stdio.h>
#include <Windows.h>

using namespace std;

// Constants for console screen size and map dimensions
const int nScreenWidth = 120;      // Console Screen Size X (columns)
const int nScreenHeight = 40;      // Console Screen Size Y (rows)
const int nMapWidth = 32;          // World Dimensions
const int nMapHeight = 32;

// Player state
float fPlayerX = 16.0f;            // Player Start Position X
float fPlayerY = 16.0f;            // Player Start Position Y
float fPlayerA = 0.0f;             // Player Start Rotation (angle)
float fFOV = 3.14159f / 4.0f;     // Field of View (angle)
float fDepth = 32.0f;             // Maximum rendering distance
float fSpeed = 5.0f;               // Walking Speed

/**
 * This game is a simple 3D raycasting engine simulation in a console window.
 * It uses raycasting to render a first-person perspective view of a 2D map.
 * The player can move around and rotate, and the game will display walls
 * and floors based on the player's view and position.
 */

 // 1. Main Function
int main()
{
    // 1.1. Create Screen Buffer
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    // 1.2. Create Map of World Space
    wstring map;
    map += L"################################";
    map += L"#..............................#";
    map += L"#..............................#";
    map += L"#.....####.....................#";
    map += L"#..............................#";
    map += L"#..............................#";
    map += L"#......................####....#";
    map += L"#..............................#";
    map += L"#..............................#";
    map += L"#..............................#";
    map += L"#..............................#";
    map += L"#..............................#";
    map += L"#..............................#";
    map += L"#..............####............#";
    map += L"#..............................#";
    map += L"#..............................#";
    map += L"#..............................#";
    map += L"#..............................#";
    map += L"#..............................#";
    map += L"#..............................#";
    map += L"#...................####.......#";
    map += L"#..............................#";
    map += L"#..............................#";
    map += L"#...........####...............#";
    map += L"#..............................#";
    map += L"#..............................#";
    map += L"#..............................#";
    map += L"#...####.......................#";
    map += L"#.....................####.....#";
    map += L"#..............................#";
    map += L"#..............................#";
    map += L"################################";

    // 1.3. Initialize Timing
    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    while (1)
    {
        // 1.4. Calculate Elapsed Time
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();

        // 1.5. Handle Player Rotation
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
            fPlayerA -= (fSpeed * 0.5f) * fElapsedTime;

        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
            fPlayerA += (fSpeed * 0.5f) * fElapsedTime;

        // 1.6. Handle Player Movement and Collision
        if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
        {
            fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;
            fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;
            if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
            {
                fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;
                fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;
            }
        }

        if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {
            fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;
            fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;
            if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
            {
                fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;
                fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;
            }
        }

        // 1.7. Render the Scene
        for (int x = 0; x < nScreenWidth; x++)
        {
            // 1.7.1. Calculate Ray Angle
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

            // 1.7.2. Ray Casting to Find Wall Distance
            float fStepSize = 0.1f;          // Increment size for ray casting
            float fDistanceToWall = 0.0f;
            bool bHitWall = false;           // Indicates if ray hits a wall
            bool bBoundary = false;         // Indicates if ray hits a boundary

            float fEyeX = sinf(fRayAngle);   // Unit vector for ray direction
            float fEyeY = cosf(fRayAngle);

            while (!bHitWall && fDistanceToWall < fDepth)
            {
                fDistanceToWall += fStepSize;
                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                // 1.7.2.1. Test If Ray Is Out of Bounds
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
                {
                    bHitWall = true;
                    fDistanceToWall = fDepth;
                }
                else
                {
                    // 1.7.2.2. Test If Ray Intersects a Wall
                    if (map.c_str()[nTestX * nMapWidth + nTestY] == '#')
                    {
                        bHitWall = true;

                        // 1.7.2.2.1. Check Boundary Details for Shading
                        vector<pair<float, float>> p;
                        for (int tx = 0; tx < 2; tx++)
                            for (int ty = 0; ty < 2; ty++)
                            {
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.push_back(make_pair(d, dot));
                            }

                        // Sort pairs by distance from player
                        sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) { return left.first < right.first; });

                        // Check boundaries for detail
                        float fBound = 0.007;
                        if (acos(p.at(0).second) < fBound) bBoundary = true;
                        if (acos(p.at(1).second) < fBound) bBoundary = true;
                        if (acos(p.at(2).second) < fBound) bBoundary = true;
                    }
                }
            }

            // 1.7.3. Calculate Ceiling and Floor Positions
            int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
            int nFloor = nScreenHeight - nCeiling;

            // 1.7.4. Determine Wall Shading Based on Distance
            short nShade = ' ';
            if (fDistanceToWall <= fDepth / 4.0f)       nShade = 0x2588;   // █ Very close
            else if (fDistanceToWall < fDepth / 3.0f)   nShade = 0x2593;   // ▓
            else if (fDistanceToWall < fDepth / 2.0f)   nShade = 0x2592;   // ▒
            else if (fDistanceToWall < fDepth)          nShade = 0x2591;   // ░
            else                                        nShade = ' ';      //   Too far away
            if      (bBoundary)                         nShade = ' ';      // Black out boundaries

            // 1.7.5. Render Each Row
            for (int y = 0; y < nScreenHeight; y++)
            {
                if (y <= nCeiling)
                    screen[y * nScreenWidth + x] = ' ';
                else if (y > nCeiling && y <= nFloor)
                    screen[y * nScreenWidth + x] = nShade;
                else
                {
                    // 1.7.5.1. Shade the Floor Based on Distance
                    float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
                    if      (b < 0.25) nShade = '#';
                    else if (b < 0.5)  nShade = 'x';
                    else if (b < 0.75) nShade = '.';
                    else if (b < 0.9)  nShade = '-';
                    else               nShade = ' ';
                    screen[y * nScreenWidth + x] = nShade;
                }
            }
        }

        // 1.8.1 Display Stats
        swprintf_s(screen, 80, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);


        // 1.8.2 Display Map
        for (int nx = 0; nx < nMapWidth; nx++)
            for (int ny = 0; ny < nMapWidth; ny++)
            {
                screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
            }
        screen[((int)fPlayerX + 1) * nScreenWidth + (int)fPlayerY] = 'P';


        // 1.9. Output the Frame to the Console
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
    }

    // 2.0. Clean Up
    delete[] screen;
    return 0;
}

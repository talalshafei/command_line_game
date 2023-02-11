#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <chrono>
#include <Windows.h>
#include <vector>
#include <utility>
#include <algorithm>
#include <cmath>
//#include <cstdio>
#include <stdio.h>


int nScreenWidth = 120;			// Console Screen Size X (columns)
int nScreenHeight = 40;			// Console Screen Size Y (rows)
int nMapWidth = 16;				// World Dimensions
int nMapHeight = 16;

float fPlayerX = 10.0f;			// Player Start Position
float fPlayerY = 5.0f;
float fPlayerA = 0.0f;			// Player Start Rotation
float fFOV = 3.14159f / 4.0f;	// Field of View
float fDepth = 16.0f;			// Maximum rendering distance
float fSpeed = 5.0f;			// Walking Speed


int main(){

    //Create Screen Buffer
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    std::wstring map;
    map += L"#########.......";
    map += L"#...............";
    map += L"#.......########";
    map += L"#..............#";
    map += L"#......##......#";
    map += L"#......##......#";
    map += L"#..............#";
    map += L"###............#";
    map += L"##.............#";
    map += L"#............###";
    map += L"#..............#";
    map += L"#......###.....#";
    map += L"#..............#";
    map += L"#......#########";
    map += L"#..............#";
    map += L"################";


    auto tp1 = std::chrono::system_clock::now();
    auto tp2 = std::chrono::system_clock::now();

    while (1) {

        tp2 = std::chrono::system_clock::now();
        std::chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();


        // Control
        // Handle CCW Rotation 
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
            fPlayerA -= (0.75f) *fElapsedTime;

        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
                fPlayerA += (0.75f) *fElapsedTime;

        if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
            fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
                fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }

        if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
            fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;

            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
                fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }

        for (int x = 0; x < nScreenWidth; x++) {
            // For each column, clculate the projected ray angle int world space
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x) / ((float)nScreenWidth);
            
            float fDistanceToWall = 0;
            bool bHitWall = false;
            bool bBoundary = false;

            float fEyeX = sinf(fRayAngle);
            float fEyeY = cosf(fRayAngle);

            while (!bHitWall && fDistanceToWall < fDepth) {
                
                fDistanceToWall += 0.1f;
                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);
                

                // Test if ray is out of bounds
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
                    bHitWall = true;
                    fDistanceToWall = fDepth;
                    
                }
                //TODO: Why not else if?
             
                else {
                    // Ray is inbound so test to see if the ray cell is a wall block
                    if (map[nTestY * nMapWidth + nTestX] == '#') {
                        bHitWall = true;

                        std::vector<std::pair<float, float>> p; // distance, dot-product
                        
                        // TODO: some vars don't depend on the inner loop
                        for(int tx = 0; tx<2; tx++)
                            for (int ty = 0; ty < 2; ty++) {
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX; // this one
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.push_back(std::make_pair(d, dot));
                            }

                        // Sort Pairs from closest to farthest
                        std::sort(p.begin(), p.end(),
                            [](const std::pair<float, float>& left, const std::pair<float, float>& right) {
                            return left.first < right.first;
                            });

                        float fBound = 0.01;
                        if(acos(p.at(0).second) < fBound) bBoundary = true;
                        if(acos(p.at(1).second) < fBound) bBoundary = true;
                            

                    }

                    
                }
            }

            // calculate distance to ceiling and floor
            int nCeiling = (floor)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
            int nFloor = nScreenHeight - nCeiling;

            short nShade = ' ';

            // TODO: switch statement
            if (fDistanceToWall <= fDepth / 4.0f) nShade = 0x2588;
            else if (fDistanceToWall < fDepth / 3.0f) nShade = 0x2593;
            else if (fDistanceToWall < fDepth / 3.0f) nShade = 0x2592;
            else if (fDistanceToWall < fDepth ) nShade = 0x2591;
            else  nShade = ' ';

            if (bBoundary) nShade = ' '; 

            for (int y = 0; y < nScreenHeight; y++) {
                if (y <= nCeiling)
                    screen[y * nScreenWidth + x] = ' ';

                else if (y > nCeiling && y <= nFloor)
                    screen[y * nScreenWidth + x] = nShade;

                else {
                    // Shade floor based on distance
                    float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));

                    // TODO: seitch statement
                    if (b < 0.25) nShade = '#';
                    else if (b < 0.5) nShade = 'x';
                    else if (b < 0.75) nShade = '.';
                    else if (b < 0.9) nShade = '-';
                    else nShade = ' ';

                    screen[y * nScreenWidth + x] = nShade;
                    
                }

            }
        }
        // TODO: find a better way to display stats
        // Display Stats
        wchar_t* const stats = new wchar_t[50];
        swprintf_s(stats, 50, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f", fPlayerX, fPlayerY, fPlayerA, (1.0f / fElapsedTime));
        swprintf_s(screen, wcslen(stats)+1, stats);
        delete[]stats;

        // Display Map
        for (int nx = 0; nx < nMapWidth; nx++)
            for (int ny = 0; ny < nMapHeight; ny++)
                screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
        

        screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 'P';
        screen[nScreenWidth * nScreenHeight - 1] = '\0';
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);


    }

    return 0;
}

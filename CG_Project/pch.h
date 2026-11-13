#pragma once

#define _CRT_SECURE_NO_WARNINGS											//--- 프로그램 맨 앞에 선언할 것
// Winsock2 must precede the Windows headers pulled in by OpenGL.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "Common.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <random>
#include <vector>
#include <math.h>
#include <cmath>
#include <queue>
#include "include/GL/glew.h"
#include "include/GL/freeglut.h"
#include "include/GL/freeglut_ext.h"

#include "include/GL/glm/glm.hpp"
#include "include/GL/glm/ext.hpp"
#include "include/GL/glm/gtc/type_ptr.hpp"
#include "include/GL/glm/gtc/matrix_transform.hpp"

#pragma comment(lib, "lib/glew32.lib")
#pragma comment(lib, "lib/freeglut.lib")

extern std::random_device rd;
extern std::uniform_real_distribution<float> random_revolve_speed;
extern std::uniform_real_distribution<float> random_obstacle;
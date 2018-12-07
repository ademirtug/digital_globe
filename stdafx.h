#pragma once

#include "targetver.h"


#include <tchar.h>
#include <iostream>
#include <string>
#include <initializer_list>
#include <memory>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <windows.h>
#include <set>
#include <windows.h>
#include <algorithm>
#include <math.h>
#include <gdiplus.h>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <queue>


#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/euler_angles.hpp"


#include "glew\include\GL\glew.h"
#include "GLFW\glfw3.h"


#include "shader.h"
#include "texture.h"
#include "mesh.h"
#include "camera.h"
#include "light.h"
#include "scene.h"
#include "engine.h"
#include "http_client.h"


#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"winhttp.lib")
#pragma comment(lib,"wininet.lib")
#pragma comment (lib, "gdiplus.lib")
#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "opengl32.lib")

#pragma once

#include <iostream>
using namespace std;



#define __FILENAME__ ((__FILE__) + SOURCE_PATH_SIZE)

#define var(x) cout << __FILENAME__ << ":" << __LINE__ << " - " << #x << " = " << x << "\n";
#define varr(x) cout << __FILENAME__ << ":" << __LINE__ << " - " << #x << " = " << x << " ";
#define line(x) cout << __FILENAME__ << ":" << __LINE__ << " - " << x << "\n";
#define linee(x) cout << __FILENAME__ << ":" << __LINE__ << " - " << x << " ";
#define nline cout << "\n";
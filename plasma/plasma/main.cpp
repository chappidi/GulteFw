// plasma.cpp : Defines the entry point for the application.
//

#include "plasma.h"
#include <plasma_client/NewOrderSingle.h>
using namespace std;

int main()
{
	cout << "Hello CMake." << plasma::client::NewOrderSingle::sbeSchemaVersion() << endl;
	return 0;
}

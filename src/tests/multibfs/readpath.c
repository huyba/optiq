#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <stdlib.h>
#include <stdio.h>

using namespace std;

int main (int argc, char *argv[]) 
{
    ifstream in_stream;

    string line;

    in_stream.open(argv[1]);

    while(!in_stream.eof())
    {
	in_stream >> line;
	printf("%s\n", line.c_str());
    }

    in_stream.close();
}


#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <fstream>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

using namespace std;

int main()
{
	using namespace transport_catalogue::catalogue;
	using namespace transport_catalogue::reader;
	using namespace transport_catalogue::stat_reader;
	std::ofstream out("out.txt");
	std::streambuf* coutbuf = std::cout.rdbuf();
	std::cout.rdbuf(out.rdbuf()); 
	ifstream inFile;

	inFile.open("tsC_case1_input.txt");

	string s;
	getline(inFile, s);

	int num_of_queries = stoi(s);
	
	Reader reader;
	
	for (int i = 0; i < num_of_queries; ++i)
	{
		reader.ReadQuery(inFile);
	}
	TransportCatalogue tc(reader);
	
	getline(inFile, s);
	num_of_queries = stoi(s);
	for (int i = 0; i < num_of_queries; ++i)
	{
		reader.ReadQuery(inFile);
	}
	ProcessInfoQueries(tc, reader);
	inFile.close();
	ifstream output;
	output.open("out.txt");
	inFile.open("tsC_case1_output1.txt");
	string s1, s2;
	std::cout.rdbuf(coutbuf);
	
	// count lines in files
	size_t n1 = std::count(std::istreambuf_iterator<char>(inFile), std::istreambuf_iterator<char>(), '\n');
	size_t n2 = std::count(std::istreambuf_iterator<char>(output), std::istreambuf_iterator<char>(), '\n');
	if (n1 != n2)
	{
		cout << "numbers of lines are not equal" << endl;
		cout << "in out: " << n2 << "\t" << "in check: " << n1 << endl;
		return 0;
	}
	inFile.clear();
	inFile.seekg(0);
	output.clear();
	output.seekg(0);
	while (n1 || n2)
	{
		--n1; --n2;
		getline(output, s1);
		getline(inFile, s2);
		if (s1 != s2)
		{
			cout << "output: " << s1 << "\t should be: " << s2 << endl;
			return 0;
		}
	}
	cout << "All good" << endl;

}
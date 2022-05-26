#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <fstream>

#include "transport_catalogue.h"
#include "request_handler.h"
#include "svg.h"
#include "domain.h"

using namespace std;

int main()
{
	using namespace transport_catalogue::catalogue;
	using namespace transport_catalogue::reader;
	using namespace transport_catalogue::renderer;
	using namespace transport_catalogue;
	std::ifstream in("input.json");
	std::streambuf* cinbuf = std::cin.rdbuf();
	std::cin.rdbuf(in.rdbuf());

	//std::ofstream out("out.txt");
	//std::streambuf* coutbuf = std::cout.rdbuf();
	//std::cout.rdbuf(out.rdbuf());
	JsonReader reader;
	MapRenderer renderer;
	
	reader.ParseRequest(std::cin, renderer);
	TransportCatalogue tc(&reader);
	RequestHandler request_handler(tc, reader, renderer);
	cout << request_handler.ProcessInfoAsJson();

}

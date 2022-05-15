#include "input_reader.h"

namespace transport_catalogue
{

	namespace reader
	{

		std::pair<std::string, double> Reader::ParseDistToStop(const std::string& query)
		{
			size_t dist_end = query.find('m');
			double dist = stod(query.substr(0, dist_end));
			size_t stop_name_start = query.find("to", dist_end) + 2;
			std::string stop_name = query.substr(stop_name_start);
			util::Trim(stop_name);
			return { std::move(stop_name), dist };
		}

		void Reader::ReadQuery(std::istream& in)
		{
			std::string s;
			std::getline(in, s);
			size_t start = s.find_first_not_of(" \t");

			if ((s[start] == 'S') && (s.find(':') != std::string::npos))
			{
				StopQuery stop;
				size_t name_start = s.find_first_not_of(" \t", s.find(" ", start));
				size_t name_end = s.find(':', name_start);
				stop.stop_name = s.substr(name_start, name_end - name_start);
				util::Trim(stop.stop_name);
				size_t coor_start1 = s.find_first_not_of(" \t:", name_end + 1);
				size_t coor_end1 = s.find_first_of(" \t,", coor_start1);
				stop.coordinates.lat = stod(s.substr(coor_start1, (coor_end1 - coor_start1)));
				size_t coor_start2 = s.find_first_not_of(" \t,", coor_end1);
				size_t coor_end2 = s.find(',', coor_start2);
				if (coor_end2 == std::string::npos)
				{
					coor_end2 = s.find_last_not_of(" \t");
					stop.coordinates.lng = stod(s.substr(coor_start2, (coor_end2 - coor_start2 + 1)));
					stop_queries_.push_back(std::move(stop));
				}
				else
				{
					stop.coordinates.lng = stod(s.substr(coor_start2, (coor_end2 - coor_start2)));
					size_t stop_dist_start = coor_end2;
					size_t stop_dist_end = s.find(',', stop_dist_start + 1);
					while (stop_dist_start != std::string::npos)
					{
						std::string tmp = s.substr(stop_dist_start + 1, (stop_dist_end - stop_dist_start - 1));
						util::Trim(tmp);
						stop.stop_to_distance.insert(ParseDistToStop(tmp));
						stop_dist_start = stop_dist_end;
						stop_dist_end = s.find(',', stop_dist_start + 1);
					}
					stop_queries_.push_back(std::move(stop));

				}
			}
			else if ((s[start] == 'B') && (s.find(':') != std::string::npos))
			{
				BusQuery bus;
				size_t name_start = s.find_first_not_of(" \t", s.find(" ", start));
				size_t name_end = s.find(':', name_start);
				bus.bus_name = s.substr(name_start, name_end - name_start);
				util::Trim(bus.bus_name);

				size_t start_stop = s.find_first_not_of(" \t", name_end + 1) - 1;
				size_t end_stop = start_stop;
				while (s.find('-', end_stop + 1) != std::string::npos)
				{
					end_stop = s.find('-', end_stop + 1);
					std::string stop = s.substr(start_stop, (end_stop - start_stop));
					util::Trim(stop);
					bus.stops.push_back(std::move(stop));
					start_stop = end_stop + 1;
				}
				if (s.find('-') != std::string::npos)
				{
					std::string stop = s.substr(start_stop);
					util::Trim(stop);
					bus.stops.push_back(std::move(stop));
					bus.stops.reserve(2 * bus.stops.size());
					bus.stops.insert(bus.stops.end(), bus.stops.rbegin() + 1, bus.stops.rend());

					bus_queries_.push_back(std::move(bus));
					return;
				}

				while (s.find('>', end_stop + 1) != std::string::npos)
				{
					end_stop = s.find('>', end_stop + 1);
					std::string stop = s.substr(start_stop, (end_stop - start_stop));
					util::Trim(stop);
					bus.stops.push_back(std::move(stop));
					start_stop = end_stop + 1;
				}
				std::string stop = s.substr(start_stop);
				util::Trim(stop);
				bus.stops.push_back(std::move(stop));

				bus_queries_.push_back(std::move(bus));
			}
			else if (s[start] == 'B')
			{
				size_t start_bus_name = s.find_first_not_of(" \t", s.find_first_of(" \t", start));
				std::string bus_name = s.substr(start_bus_name);
				util::Rtrim(bus_name);
				info_queries_.push_back(InfoQuery(std::move(bus_name), false));
			}
			else if (s[start] == 'S')
			{
				size_t start_stop_name = s.find_first_not_of(" \t", s.find_first_of(" \t", start));
				std::string stop_name = s.substr(start_stop_name);
				util::Rtrim(stop_name);
				info_queries_.push_back(InfoQuery(std::move(stop_name), true));
			}
		}

		void Reader::PrintStopQueries()
		{
			for (auto& q : stop_queries_)
			{
				std::cout << "query " << q.stop_name << ";" << " latitude: " << std::setprecision(8) << q.coordinates.lat << ", longitude: "
					<< q.coordinates.lng << std::endl;
			}
		}

		void Reader::PrintBusQueries()
		{
			for (auto& q : bus_queries_)
			{
				std::cout << "BUS NAME: " << q.bus_name << std::endl;
				int i = 0;
				for (auto& s : q.stops)
				{
					std::cout << "STOP " << ++i << ": " << s << std::endl;
				}
			}
		}

	}

}
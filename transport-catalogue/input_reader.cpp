#include "input_reader.h"

namespace transport_catalogue
{

	namespace reader
	{
		StopQuery::StopQuery(StopQuery&& other) noexcept
		{
			stop_name = std::move(other.stop_name);
			stop_to_distance = std::move(other.stop_to_distance);
			coordinates.lat = other.coordinates.lat;
			coordinates.lng = other.coordinates.lng;
		}

		BusQuery::BusQuery(BusQuery&& other) noexcept
		{
			bus_name = std::move(other.bus_name);
			stops = std::move(other.stops);
		}

		InfoQuery::InfoQuery(std::string&& name_, bool is_stop_)
			: name(name_), is_stop(is_stop_)
		{

		}

		std::pair<std::string, double> Reader::ParseDistToStop(const std::string& query)
		{
			size_t dist_end = query.find('m');
			double dist = stod(query.substr(0, dist_end));
			size_t stop_name_start = query.find("to", dist_end) + 2;
			std::string stop_name = query.substr(stop_name_start);
			util::Trim(stop_name);
			return { std::move(stop_name), dist };
		}
		StopQuery Reader::ParseStopAddQuery(const std::string& s)
		{
			StopQuery stop;
			size_t name_start = s.find_first_not_of(" \t", s.find(" "));
			size_t name_end = s.find(':', name_start);
			stop.stop_name = s.substr(name_start, name_end - name_start);
			util::Trim(stop.stop_name);
			size_t coor_start1 = s.find_first_not_of(" \t:", name_end + 1);
			size_t coor_end1 = s.find_first_of(" \t,", coor_start1);
			stop.coordinates.lat = stod(s.substr(coor_start1, (coor_end1 - coor_start1)));
			size_t coor_start2 = s.find_first_not_of(" \t,", coor_end1);
			size_t coor_end2 = s.find(',', coor_start2);

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
			return stop;
		}
		BusQuery Reader::ParseBusAddChainQuery(const std::string& s)
		{
			BusQuery bus;
			size_t name_start = s.find_first_not_of(" \t", s.find(" "));
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
			std::string stop = s.substr(start_stop);
			util::Trim(stop);
			bus.stops.push_back(std::move(stop));
			bus.stops.reserve(2 * bus.stops.size());
			bus.stops.insert(bus.stops.end(), bus.stops.rbegin() + 1, bus.stops.rend());
			return bus;
		}

		BusQuery Reader::ParseBusAddCycleQuery(const std::string& s)
		{
			BusQuery bus;
			size_t name_start = s.find_first_not_of(" \t", s.find(" "));
			size_t name_end = s.find(':', name_start);
			bus.bus_name = s.substr(name_start, name_end - name_start);
			util::Trim(bus.bus_name);

			size_t start_stop = s.find_first_not_of(" \t", name_end + 1) - 1;
			size_t end_stop = start_stop;
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
			return bus;
		}

		InfoQuery Reader::ParseBusInfoQuery(const std::string& s)
		{
			size_t start_bus_name = s.find_first_not_of(" \t", s.find_first_of(" \t"));
			std::string bus_name = s.substr(start_bus_name);
			util::Rtrim(bus_name);
			return { std::move(bus_name), false };
		}

		InfoQuery Reader::ParseStopInfoQuery(const std::string& s)
		{
			size_t start_stop_name = s.find_first_not_of(" \t", s.find_first_of(" \t"));
			std::string stop_name = s.substr(start_stop_name);
			util::Rtrim(stop_name);
			return { std::move(stop_name), true };
		}

		void Reader::ReadQuery(std::istream& in)
		{
			std::string s;
			std::getline(in, s);
			size_t start = s.find_first_not_of(" \t");

			if ((s[start] == 'S') && (s.find(':') != std::string::npos))
			{
				stop_queries_.push_back(ParseStopAddQuery(s));
			}
			else if ((s[start] == 'B') && (s.find(':') != std::string::npos))
			{
				if (s.find('-') != std::string::npos)
				{
					bus_queries_.push_back(ParseBusAddChainQuery(s));
					return;
				}
				if (s.find('>') != std::string::npos)
				{
					bus_queries_.push_back(ParseBusAddCycleQuery(s));
					return;
				}
			}
			else if (s[start] == 'B')
			{
				info_queries_.push_back(ParseBusInfoQuery(s));
			}
			else if (s[start] == 'S')
			{
				info_queries_.push_back(ParseStopInfoQuery(s));
			}
		}

		void Reader::PrintStopQueries(std::ostream& out)
		{
			for (auto& q : stop_queries_)
			{
				out << "query " << q.stop_name << ";" << " latitude: " << std::setprecision(8) << q.coordinates.lat << ", longitude: "
					<< q.coordinates.lng << std::endl;
			}
		}

		void Reader::PrintBusQueries(std::ostream& out)
		{
			for (auto& q : bus_queries_)
			{
				out << "BUS NAME: " << q.bus_name << std::endl;
				int i = 0;
				for (auto& s : q.stops)
				{
					out << "STOP " << ++i << ": " << s << std::endl;
				}
			}
		}

	}

}
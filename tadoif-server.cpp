#include "tadoif.h"

#include <iostream>
#include <string>
#include <boost/format.hpp>

int main(int argc, const char **argv)
{
	try
	{
		TadoIf tado_if;
		TadoIf::Data data;
		int zone_nr;
		char timestring[64];
		const struct tm *tm;
		time_t timestamp;

		data = tado_if.fetch(argc, argv);
		zone_nr = 0;

		std::cout << boost::format("%-5s %-2s %-20s %-8s %-5s %-11s %-8s %s\n") %
				"zone#" % "id" % "name" % "status" % "power" % "temperature" % "humidity" % "time";

		for(const auto &zone : data.zones)
		{
			timestamp = zone.time;
			tm = localtime(&timestamp);
			strftime(timestring, sizeof(timestring), "%Y-%m-%d %H:%M:%S", tm);

			std::cout << boost::format("%5u %2d %-20s %-8s %5.0f %11.1f %8.0f %s\n") %
					zone_nr %
					zone.id %
					zone.name %
					(zone.active ? "active" : "inactive") %
					(zone.power * 100) %
					zone.temperature %
					zone.humidity %
					timestring;

			zone_nr++;
		}
	}
	catch(const TadoIf::Exception &e)
	{
		std::cerr << e.what() << std::endl;
		return(-1);
	}
	catch(const std::exception &e)
	{
		std::cerr << "standard exception: " << e.what() << std::endl;
		return(-1);
	}
	catch(...)
	{
		std::cerr << "unknown exception" << std::endl;
		return(-1);
	}

	return(0);
}
